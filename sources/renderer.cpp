#include "renderer.hpp"

#include "printer.hpp"

#include <cassert>
#include <string>

namespace Limcore
{
	Result Renderer::Create(const VkDevice& logicalDevice, const uint32_t& queueFamilyIndex, RendererConfig rendererConfig)
	{
		assert(logicalDevice != nullptr);
		assert(rendererConfig.maxFramesInFlight >= 1 && rendererConfig.maxFramesInFlight <= 3);

		const std::string message = "Failed to create renderer";

		this->logicalDevice = logicalDevice;
		config = rendererConfig;

		Result result = CreateFences(frameFences, logicalDevice, config.maxFramesInFlight);
		RETURN_ERROR(result, message)

		result = CreateSemaphores(canRenderSemaphores, logicalDevice, config.maxFramesInFlight);
		RETURN_ERROR(result, message)

		result = CreateCommandPools(commandPools, logicalDevice, queueFamilyIndex, config.maxFramesInFlight);
		RETURN_ERROR(result, message)

		result = AllocateCommandBuffers();
		RETURN_ERROR(result, message)

		return (Result());
	}

	Result Renderer::AllocateCommandBuffers()
	{
		assert(commandBuffers.empty());
		assert(commandPools.size() == config.maxFramesInFlight);

		commandBuffers.resize(config.maxFramesInFlight);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.pNext = nullptr;

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			allocateInfo.commandPool = commandPools[i];
			VkResult result = vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffers[i]);
			if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to allocate command buffers", result}));}
		}

		if (config.log) {std::cout << "Renderer command buffers allocated" << std::endl;}

		return (Result());
	}

	void Renderer::Destroy() noexcept
	{
		if (logicalDevice == nullptr) {return;}

		vkDeviceWaitIdle(logicalDevice);

		if (!frameFences.empty())
		{
			for (VkFence& fence : frameFences) {vkDestroyFence(logicalDevice, fence, nullptr);}
			frameFences.clear();
			if (config.log) {std::cout << "Renderer fences destroyed" << std::endl;}
		}

		if (!canRenderSemaphores.empty())
		{
			for (VkSemaphore& semaphore : canRenderSemaphores) {vkDestroySemaphore(logicalDevice, semaphore, nullptr);}
			canRenderSemaphores.clear();
			if (config.log) {std::cout << "Renderer semaphores destroyed" << std::endl;}
		}

		if (!commandPools.empty())
		{
			for (VkCommandPool& commandPool : commandPools) {vkDestroyCommandPool(logicalDevice, commandPool, nullptr);}
			commandPools.clear();
			if (config.log) {std::cout << "Renderer command pools destroyed" << std::endl;}
		}

		if (!commandBuffers.empty()) {commandBuffers.clear();}

		logicalDevice = nullptr;
	}

	Result Renderer::WaitForFrame()
	{
		assert(logicalDevice != nullptr);
		assert(!frameFences.empty());

		if (state != RendererState::Presented)
			{return (std::unexpected(Error(ErrorCode::Unknown, std::string("Renderer state is not Presented: ").append(EnumName(state)))));}

		Result result = WaitForFence(frameFences[frameIndex], logicalDevice, FRAME_FENCE_TIMEOUT);
		RETURN_ERROR(result, "Failed to wait for frame")

		state = RendererState::Waited;

		return (Result());
	}

	Result Renderer::BeginFrame(const VkSwapchainKHR& swapchain)
	{
		assert(logicalDevice != nullptr);
		assert(swapchain != nullptr);

		if (state != RendererState::Waited)
			{return (std::unexpected(Error(ErrorCode::Unknown, std::string("Renderer state is not Waited: ").append(EnumName(state)))));}

		swapchainOutOfDate = false;

		presentIndex = NO_PRESENT_IMAGE;
		VkResult result = vkAcquireNextImageKHR(logicalDevice, swapchain, ACQUIRE_IMAGE_TIMEOUT, canRenderSemaphores[frameIndex], nullptr, &presentIndex);
		if (IsSwapchainError(result)) {swapchainOutOfDate = true;}
		if (result != VK_SUBOPTIMAL_KHR) {RETURN_VK_ERROR(result, "Failed to acquire next image")}
		
		result = vkResetCommandPool(logicalDevice, commandPools[frameIndex], 0);
		RETURN_VK_ERROR(result, "Failed to reset command pool")

		Result beginning = BeginCommand(commandBuffers[frameIndex]);
		if (!beginning) {return (beginning);}

		state = RendererState::Began;

		return (Result());
	}

	Result Renderer::EndFrame(const VkQueue& submitQueue, const std::vector<VkSemaphore>& canPresentSemaphores, std::vector<VkSemaphoreSubmitInfo> waitInfos, std::vector<VkSemaphoreSubmitInfo> signalInfos)
	{
		assert(submitQueue != nullptr);
		assert(presentIndex < canPresentSemaphores.size());

		if (state != RendererState::Began)
			{return (std::unexpected(Error(ErrorCode::Unknown, std::string("Renderer state is not Began: ").append(EnumName(state)))));}

		Result result = EndCommand(commandBuffers[frameIndex]);
		RETURN_ERROR(result, "Failed to end frame")

		state = RendererState::Ended;

		VkSemaphoreSubmitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.semaphore = canRenderSemaphores[frameIndex];
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		waitInfo.pNext = nullptr;

		VkSemaphoreSubmitInfo signalInfo{};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.semaphore = canPresentSemaphores[presentIndex];
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		signalInfo.pNext = nullptr;

		waitInfos.push_back(waitInfo);
		signalInfos.push_back(signalInfo);

		result = SubmitCommand(commandBuffers[frameIndex], submitQueue, waitInfos, signalInfos, frameFences[frameIndex]);
		RETURN_ERROR(result, "Failed to end frame")

		state = RendererState::Submitted;

		return (Result());
	}

	Result Renderer::PresentFrame(const VkSwapchainKHR& swapchain, const std::vector<VkSemaphore>& canPresentSemaphores, const VkQueue& presentQueue)
	{
		assert(swapchain != nullptr);
		assert(presentIndex < canPresentSemaphores.size());
		assert(presentQueue != nullptr);

		if (state != RendererState::Submitted)
			{return (std::unexpected(Error(ErrorCode::Unknown, std::string("Renderer state is not Submitted: ").append(EnumName(state)))));}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &presentIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &canPresentSemaphores[presentIndex];
		presentInfo.pNext = nullptr;

		VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (IsSwapchainError(result)) {swapchainOutOfDate = true;}
		else {RETURN_VK_ERROR(result, "Failed to present to queue")}

		frameIndex = (frameIndex + 1) % config.maxFramesInFlight;

		state = RendererState::Presented;

		return (Result());
	}
}
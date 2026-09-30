#include "renderer.hpp"

#include <cassert>

namespace Limcore
{
	Result<void> Renderer::Create(const VkDevice& logicalDevice, const uint32_t& queueFamilyIndex, RendererConfig rendererConfig)
	{
		assert(logicalDevice != nullptr);
		assert(rendererConfig.maxFramesInFlight >= 1 && rendererConfig.maxFramesInFlight <= 3);

		this->logicalDevice = logicalDevice;
		config = rendererConfig;

		Result<void> fencesCreation = CreateFences();
		if (!fencesCreation) {return (std::unexpected(Error(fencesCreation.error(), "Failed to create renderer")));}

		Result<void> semaphoresCreation = CreateSemaphores();
		if (!semaphoresCreation) {return (std::unexpected(Error(semaphoresCreation.error(), "Failed to create renderer")));}

		Result<void> commandPoolsCreation = CreateCommandPools(queueFamilyIndex);
		if (!commandPoolsCreation) {return (std::unexpected(Error(commandPoolsCreation.error(), "Failed to create renderer")));}

		Result<void> commandBuffersAllocation = AllocateCommandBuffers();
		if (!commandBuffersAllocation) {return (std::unexpected(Error(commandBuffersAllocation.error(), "Failed to create renderer")));}

		return (Result<void>());
	}

	Result<void> Renderer::CreateFences()
	{
		assert(frameFences.empty());

		frameFences.resize(config.maxFramesInFlight);

		VkFenceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < frameFences.size(); i++)
		{
			VkResult result = vkCreateFence(logicalDevice, &createInfo, nullptr, &frameFences[i]);
			if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create fences", result}));}
		}

		if (config.log) {std::cout << "Renderer fences created" << std::endl;}

		return (Result<void>());
	}

	Result<void> Renderer::CreateSemaphores()
	{
		assert(canRenderSemaphores.empty());

		canRenderSemaphores.resize(config.maxFramesInFlight);

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (size_t i = 0; i < canRenderSemaphores.size(); i++)
		{
			VkResult result = vkCreateSemaphore(logicalDevice, &createInfo, nullptr, &canRenderSemaphores[i]);
			if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create semaphores", result}));}
		}

		if (config.log) {std::cout << "Renderer semaphores created" << std::endl;}

		return (Result<void>());
	}

	Result<void> Renderer::CreateCommandPools(const uint32_t& queueFamilyIndex)
	{
		assert(commandPools.empty());

		commandPools.resize(config.maxFramesInFlight);

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndex;
		createInfo.pNext = nullptr;

		for (size_t i = 0; i < commandPools.size(); i++)
		{
			VkResult result = vkCreateCommandPool(logicalDevice, &createInfo, nullptr, &commandPools[i]);
			if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create command pools", result}));}
		}

		if (config.log) {std::cout << "Renderer command pools created" << std::endl;}

		return (Result<void>());
	}

	Result<void> Renderer::AllocateCommandBuffers()
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

		return (Result<void>());
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

	Result<void> Renderer::WaitForFrame()
	{
		assert(logicalDevice != nullptr);
		assert(!frameFences.empty());

		VkResult result = vkWaitForFences(logicalDevice, 1, &frameFences[frameIndex], true, FRAME_FENCE_TIMEOUT);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to wait for fence", result}));}

		result = vkResetFences(logicalDevice, 1, &frameFences[frameIndex]);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to reset fence", result}));}

		return (Result<void>());
	}

	Result<void> Renderer::RecordCommands(const VkSwapchainKHR& swapchain, const std::vector<VkImage>& swapchainImages, const std::vector<VkImageView>& swapchainViews, const VkExtent2D& swapchainExtent, const std::vector<VkSemaphore>& canPresentSemaphores, const VkQueue& graphicsQueue, const VkQueue& presentQueue, bool start)
	{
		assert(logicalDevice != nullptr);
		assert(swapchain != nullptr);
		assert(!swapchainImages.empty());
		assert(!swapchainViews.empty());
		assert(!canPresentSemaphores.empty());
		assert(graphicsQueue != nullptr);
		assert(presentQueue != nullptr);

		uint32_t presentImageIndex;
		VkResult result = vkAcquireNextImageKHR(logicalDevice, swapchain, ACQUIRE_IMAGE_TIMEOUT, canRenderSemaphores[frameIndex], nullptr, &presentImageIndex);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to acquire next image", result}));}

		result = vkResetCommandPool(logicalDevice, commandPools[frameIndex], 0);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to reset command pool", result}));}

		VkCommandBufferBeginInfo commandBeginInfo{};
		commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBeginInfo.pInheritanceInfo = nullptr;
		commandBeginInfo.pNext = nullptr;

		result = vkBeginCommandBuffer(commandBuffers[frameIndex], &commandBeginInfo);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to begin command buffer", result}));}

		TransitionToColor(swapchainImages[presentImageIndex], start);

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = swapchainViews[presentImageIndex];
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {{float(frameIndex), 0.0f, 0.0f, 1.0f}};

		VkRenderingInfo renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = {{0, 0}, swapchainExtent};
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		vkCmdBeginRendering(commandBuffers[frameIndex], &renderInfo);
		vkCmdEndRendering(commandBuffers[frameIndex]);

		TransitionToPresent(swapchainImages[presentImageIndex]);

		result = vkEndCommandBuffer(commandBuffers[frameIndex]);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to end command buffer", result}));}

		VkCommandBufferSubmitInfo commandSubmitInfo{};
		commandSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		commandSubmitInfo.commandBuffer = commandBuffers[frameIndex];
		commandSubmitInfo.pNext = nullptr;

		VkSemaphoreSubmitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.semaphore = canRenderSemaphores[frameIndex];
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		waitInfo.pNext = nullptr;

		VkSemaphoreSubmitInfo signalInfo{};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.semaphore = canPresentSemaphores[presentImageIndex];
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		signalInfo.pNext = nullptr;

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &commandSubmitInfo;
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		submitInfo.pNext = nullptr;

		result = vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frameFences[frameIndex]);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to submit to queue", result}));}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &presentImageIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &canPresentSemaphores[presentImageIndex];
		presentInfo.pNext = nullptr;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to present to queue", result}));}

		frameIndex = (frameIndex + 1) % config.maxFramesInFlight;

		return (Result<void>());
	}

	void Renderer::TransitionToColor(const VkImage& image, bool undefined)
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier.srcAccessMask = 0;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = (undefined ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.imageMemoryBarrierCount = 1;
		dependency.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(commandBuffers[frameIndex], &dependency);
	}

	void Renderer::TransitionToPresent(const VkImage& image)
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.imageMemoryBarrierCount = 1;
		dependency.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(commandBuffers[frameIndex], &dependency);
	}
}
#include "command.hpp"

#include "utility.hpp"

#include <cassert>

namespace Limcore
{
	/*Result Command::Create(const VkDevice& logicalDevice, const VkCommandPool& pool, const VkQueue& submitQueue, VkFence fence, CommandConfig commandConfig)
	{
		assert(logicalDevice != nullptr);
		assert(pool != nullptr);
		assert(submitQueue != nullptr);

		config = commandConfig;
		this->submitQueue = submitQueue;
		this->fence = fence;

		Result allocation = Allocate(logicalDevice, pool);
		if (!allocation) {return (std::unexpected(Error(allocation.error(), "Failed to create command")));}

		return (Result());
	}

	Result Command::Allocate(const VkDevice& logicalDevice, const VkCommandPool& pool)
	{
		assert(commandBuffer == nullptr);
		assert(logicalDevice != nullptr);
		assert(pool != nullptr);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = pool;
		allocateInfo.level = config.level;
		allocateInfo.pNext = nullptr;

		VkResult result = vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffer);
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to allocate command buffers", result}));}

		return (Result());
	}

	void Command::Destroy() noexcept
	{
		state = CommandState::Idle;
		config = {};
		commandBuffer = nullptr;
		waitInfos.clear();
		signalInfos.clear();
		submitQueue = nullptr;
	}

	void Command::AddWaitSemaphore(const VkSemaphore& semaphore, VkPipelineStageFlags2 stages)
	{
		assert(semaphore != nullptr);
		
	}*/

	Result CreateFence(VkFence& fence, const VkDevice& logicalDevice, VkFenceCreateFlags flags)
	{
		assert(fence == nullptr);
		assert(logicalDevice != nullptr);

		VkFenceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = flags;

		VkResult result = vkCreateFence(logicalDevice, &createInfo, nullptr, &fence);
		RETURN_VK_ERROR(result, "Failed to create fence")
	
		return (Result());
	}

	Result CreateFences(std::vector<VkFence>& fences, const VkDevice& logicalDevice, uint32_t count, VkFenceCreateFlags flags)
	{
		assert(fences.empty());
		assert(logicalDevice != nullptr);
		assert(count > 0);

		fences.resize(count);

		for (VkFence& fence : fences)
		{
			Result fenceCreation = CreateFence(fence, logicalDevice, flags);
			if (!fenceCreation) {return (fenceCreation);}
		}

		return (Result());
	}

	Result CreateSemaphore(VkSemaphore& semaphore, const VkDevice& logicalDevice)
	{
		assert(semaphore == nullptr);
		assert(logicalDevice != nullptr);

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkResult result = vkCreateSemaphore(logicalDevice, &createInfo, nullptr, &semaphore);
		RETURN_VK_ERROR(result, "Failed to create semaphore")

		return (Result());
	}

	Result CreateSemaphores(std::vector<VkSemaphore>& semaphores, const VkDevice& logicalDevice, uint32_t count)
	{
		assert(semaphores.empty());
		assert(logicalDevice != nullptr);
		assert(count > 0);

		semaphores.resize(count);

		for (VkSemaphore& semaphore : semaphores)
		{
			Result semaphoreCreation = CreateSemaphore(semaphore, logicalDevice);
			if (!semaphoreCreation) {return (semaphoreCreation);}
		}

		return (Result());
	}

	Result CreateCommandPool(VkCommandPool& commandPool, const VkDevice& logicalDevice, const uint32_t& queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		assert(commandPool == nullptr);
		assert(logicalDevice != nullptr);
		assert(queueFamilyIndex != NO_QUEUE_FAMILY);

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = flags;
		createInfo.queueFamilyIndex = queueFamilyIndex;
		createInfo.pNext = nullptr;

		VkResult result = vkCreateCommandPool(logicalDevice, &createInfo, nullptr, &commandPool);
		RETURN_VK_ERROR(result, "Failed to create command pool")

		return (Result());
	}

	Result CreateCommandPools(std::vector<VkCommandPool>& commandPools, const VkDevice& logicalDevice, const uint32_t& queueFamilyIndex, uint32_t count, VkCommandPoolCreateFlags flags)
	{
		assert(commandPools.empty());
		assert(logicalDevice != nullptr);
		assert(queueFamilyIndex != NO_QUEUE_FAMILY);
		assert(count > 0);

		commandPools.resize(count);

		for (VkCommandPool& commandPool : commandPools)
		{
			Result commandPoolCreation = CreateCommandPool(commandPool, logicalDevice, queueFamilyIndex, flags);
			if (!commandPoolCreation) {return (commandPoolCreation);}
		}

		return (Result());
	}

	Result AllocateCommandBuffer(VkCommandBuffer& commandBuffer, const VkCommandPool& commandPool, const VkDevice& logicalDevice)
	{
		assert(commandBuffer == nullptr);
		assert(commandPool != nullptr);
		assert(logicalDevice != nullptr);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.pNext = nullptr;

		VkResult result = vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffer);
		RETURN_VK_ERROR(result, "Failed to allocate command buffer")

		return (Result());
	}

	Result AllocateCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, const VkCommandPool& commandPool, const VkDevice& logicalDevice, uint32_t count)
	{
		assert(commandBuffers.empty());
		assert(commandPool != nullptr);
		assert(logicalDevice != nullptr);
		assert(count > 0);

		commandBuffers.resize(count);

		for (VkCommandBuffer& commandBuffer : commandBuffers)
		{
			Result commandBufferAllocation = AllocateCommandBuffer(commandBuffer, commandPool, logicalDevice);
			if (!commandBufferAllocation) {return (commandBufferAllocation);}
		}

		return (Result());
	}

	Result BeginCommand(const VkCommandBuffer& commandBuffer, VkCommandBufferUsageFlags usage)
	{
		assert(commandBuffer != nullptr);

		VkCommandBufferBeginInfo commandBeginInfo{};
		commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBeginInfo.pInheritanceInfo = nullptr;
		commandBeginInfo.pNext = nullptr;

		VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBeginInfo);
		RETURN_VK_ERROR(result, "Failed to begin command buffer")

		return (Result());
	}

	Result EndCommand(const VkCommandBuffer& commandBuffer)
	{
		assert(commandBuffer != nullptr);

		VkResult result = vkEndCommandBuffer(commandBuffer);
		RETURN_VK_ERROR(result, "Failed to end command buffer")

		return (Result());
	}

	Result SubmitCommand(const VkCommandBuffer& commandBuffer, const VkQueue& queue, std::vector<VkSemaphoreSubmitInfo> waitInfos, std::vector<VkSemaphoreSubmitInfo> signalInfos, VkFence fence)
	{
		assert(commandBuffer != nullptr);
		assert(queue != nullptr);

		VkCommandBufferSubmitInfo commandSubmitInfo{};
		commandSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		commandSubmitInfo.commandBuffer = commandBuffer;
		commandSubmitInfo.pNext = nullptr;

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &commandSubmitInfo;
		submitInfo.waitSemaphoreInfoCount = CUI(waitInfos.size());
		submitInfo.pWaitSemaphoreInfos = (submitInfo.waitSemaphoreInfoCount == 0 ? nullptr : waitInfos.data());
		submitInfo.signalSemaphoreInfoCount = CUI(signalInfos.size());
		submitInfo.pSignalSemaphoreInfos = (submitInfo.signalSemaphoreInfoCount == 0 ? nullptr : signalInfos.data());
		submitInfo.pNext = nullptr;

		VkResult result = vkQueueSubmit2(queue, 1, &submitInfo, fence);
		RETURN_VK_ERROR(result, "Failed to submit command")

		return (Result());
	}

	Result WaitForFence(const VkFence& fence, const VkDevice& logicalDevice, uint64_t timeout)
	{
		assert(fence != nullptr);
		assert(logicalDevice != nullptr);

		VkResult result = vkWaitForFences(logicalDevice, 1, &fence, true, DEFAULT_FENCE_TIMEOUT);
		RETURN_VK_ERROR(result, "Failed to wait for fence")

		result = vkResetFences(logicalDevice, 1, &fence);
		RETURN_VK_ERROR(result, "Failed to reset fence")

		return (Result());
	}
}
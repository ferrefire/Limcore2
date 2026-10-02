#pragma once

#include "error.hpp"
#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

//Todo: decide if I want to move the synchronisation to a separate class.

namespace Limcore
{
	#define DEFAULT_FENCE_TIMEOUT 1000000000

	/*enum class CommandState {Idle, Began, Ended};

	struct CommandConfig
	{
		VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	};

	class Command
	{
		private:
			CommandState state = CommandState::Idle;
			CommandConfig config{};
			VkCommandBuffer commandBuffer = nullptr;
			VkFence fence = nullptr;
			VkQueue submitQueue = nullptr;

			[[nodiscard]] Result Allocate(const VkDevice& logicalDevice, const VkCommandPool& pool);

		public:
			Command() noexcept = default;
			~Command() noexcept {Destroy();}

			Command(const Command&) = delete;
			Command& operator=(const Command&) = delete;

			//Todo: implement move operators.

			[[nodiscard]] Result Create(const VkDevice& logicalDevice, const VkCommandPool& pool, const VkQueue& submitQueue, VkFence fence, CommandConfig commandConfig);

			void Destroy() noexcept;

			void AddWaitSemaphore(const VkSemaphore& semaphore, VkPipelineStageFlags2 stages);
			void AddSignalSemaphore(const VkSemaphore& semaphore, VkPipelineStageFlags2 stages);
	};*/

	[[nodiscard]] Result CreateFence(VkFence& fence, const VkDevice& logicalDevice, VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT);
	[[nodiscard]] Result CreateFences(std::vector<VkFence>& fences, const VkDevice& logicalDevice, uint32_t count, VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT);

	[[nodiscard]] Result CreateSemaphore(VkSemaphore& semaphore, const VkDevice& logicalDevice);
	[[nodiscard]] Result CreateSemaphores(std::vector<VkSemaphore>& semaphores, const VkDevice& logicalDevice, uint32_t count);

	[[nodiscard]] Result CreateCommandPool(VkCommandPool& commandPool, const VkDevice& logicalDevice, const uint32_t& queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	[[nodiscard]] inline Result CreateCommandPool(VkCommandPool& commandPool, const Device& device, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
		{return (CreateCommandPool(commandPool, device.GetLogicalDevice(), device.GetSelectedQueueFamily(), flags));}
	[[nodiscard]] Result CreateCommandPools(std::vector<VkCommandPool>& commandPools, const VkDevice& logicalDevice, const uint32_t& queueFamilyIndex, uint32_t count, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	[[nodiscard]] inline Result CreateCommandPools(std::vector<VkCommandPool>& commandPools, const Device& device, uint32_t count, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
		{return (CreateCommandPools(commandPools, device.GetLogicalDevice(), device.GetSelectedQueueFamily(), count, flags));}

	[[nodiscard]] Result AllocateCommandBuffer(VkCommandBuffer& commandBuffer, const VkCommandPool& commandPool, const VkDevice& logicalDevice);
	[[nodiscard]] Result AllocateCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, const VkCommandPool& commandPool, const VkDevice& logicalDevice, uint32_t count);

	[[nodiscard]] Result BeginCommand(const VkCommandBuffer& commandBuffer, VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	[[nodiscard]] Result EndCommand(const VkCommandBuffer& commandBuffer);
	[[nodiscard]] Result SubmitCommand(const VkCommandBuffer& commandBuffer, const VkQueue& queue, std::vector<VkSemaphoreSubmitInfo> waitInfos, std::vector<VkSemaphoreSubmitInfo> signalInfos, VkFence fence);

	[[nodiscard]] Result WaitForFence(const VkFence& fence, const VkDevice& logicalDevice, uint64_t timeout = DEFAULT_FENCE_TIMEOUT);
}
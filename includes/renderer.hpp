#pragma once

#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Limcore
{
	#define FRAME_FENCE_TIMEOUT 1000000000
	#define ACQUIRE_IMAGE_TIMEOUT 1000000000

	struct RendererConfig
	{
		uint32_t maxFramesInFlight = 2;
		bool log = false;
	};

	class Renderer
	{
		private:
			VkDevice logicalDevice = nullptr;

			RendererConfig config{};
			//Todo: Make struct containing all per frame resources.
			std::vector<VkFence> frameFences;
			std::vector<VkSemaphore> canRenderSemaphores;
			std::vector<VkCommandPool> commandPools;
			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t frameIndex = 0;

			[[nodiscard]] Result CreateFences();
			[[nodiscard]] Result CreateSemaphores();
			[[nodiscard]] Result CreateCommandPools(const uint32_t& queueFamilyIndex);
			[[nodiscard]] Result AllocateCommandBuffers();

			void TransitionToColor(const VkImage& image, bool undefined = false); //Todo: make a dynamic version in the image class.
			void TransitionToPresent(const VkImage& image);

		public:
			Renderer() noexcept = default;
			~Renderer() noexcept {Destroy();}

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			//Todo: implement move operators.

			[[nodiscard]] Result Create(const VkDevice& logicalDevice, const uint32_t& queueFamilyIndex, RendererConfig rendererConfig);
			[[nodiscard]] Result Create(const Device& device, RendererConfig rendererConfig)
				{return (Create(device.GetLogicalDevice(), device.GetSelectedQueueFamily(), rendererConfig));}

			void Destroy() noexcept;

			Result WaitForFrame();
			//Result BeginFrame();
			//Result EndFrame();

			Result RecordCommands(const VkSwapchainKHR& swapchain, const std::vector<VkImage>& swapchainImages, const std::vector<VkImageView>& swapchainViews, const VkExtent2D& swapchainExtent, const std::vector<VkSemaphore>& canPresentSemaphores, const VkQueue& graphicsQueue, const VkQueue& presentQueue, bool start = false);
			Result RecordCommands(const Viewport& viewport, const VkQueue& graphicsQueue, const VkQueue& presentQueue, bool start = false)
				{return (RecordCommands(viewport.GetSwapchain(), viewport.GetImages(), viewport.GetViews(), viewport.GetExtent(), viewport.GetSemaphores(), graphicsQueue, presentQueue, start));}
			Result RecordCommands(const Viewport& viewport, const Device& device, bool start = false)
				{return (RecordCommands(viewport.GetSwapchain(), viewport.GetImages(), viewport.GetViews(), viewport.GetExtent(), viewport.GetSemaphores(), device.GetQueue(QueueType::Graphics), device.GetQueue(QueueType::Present), start));}
	};
}
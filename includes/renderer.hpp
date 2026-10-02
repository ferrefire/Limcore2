#pragma once

#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "error.hpp"
#include "command.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Limcore
{
	#define FRAME_FENCE_TIMEOUT 1000000000
	#define ACQUIRE_IMAGE_TIMEOUT 1000000000
	#define NO_PRESENT_IMAGE UINT32_MAX

	enum class RendererState {Waited, Began, Ended, Submitted, Presented};

	struct RendererConfig
	{
		uint32_t maxFramesInFlight = 2;
		bool log = false;
	};

	/*struct RendererAttachmentInfo
	{
		VkRenderingAttachmentInfo attachmentInfo{};
		std::vector<VkImage
	};

	struct RendererPassInfo
	{

	};*/

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

			RendererState state = RendererState::Presented;
			uint32_t frameIndex = 0;
			uint32_t presentIndex = NO_PRESENT_IMAGE;

			[[nodiscard]] Result AllocateCommandBuffers();

			//void TransitionToColor(const VkImage& image); //Todo: make a dynamic version in the image class.
			//void TransitionToPresent(const VkImage& image);

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

			[[nodiscard]] const uint32_t& GetFrameIndex() const noexcept {return (frameIndex);}
			[[nodiscard]] const uint32_t& GetPresentIndex() const noexcept {return (presentIndex);}
			[[nodiscard]] const VkCommandBuffer& GetCommandBuffer() const {return (commandBuffers[frameIndex]);}

			Result WaitForFrame();
			Result BeginFrame(const VkSwapchainKHR& swapchain);
			Result BeginFrame(const Viewport& viewport) {return (BeginFrame(viewport.GetSwapchain()));}
			Result EndFrame(const VkQueue& graphicsQueue, const std::vector<VkSemaphore>& canPresentSemaphores, std::vector<VkSemaphoreSubmitInfo> waitInfos, std::vector<VkSemaphoreSubmitInfo> signalInfos);
			Result EndFrame(const Device& device, const Viewport& viewport) {return (EndFrame(device.GetQueue(QueueType::Graphics), viewport.GetSemaphores(), {}, {}));}
			Result PresentFrame(const VkSwapchainKHR& swapchain, const std::vector<VkSemaphore>& canPresentSemaphores, const VkQueue& presentQueue);
			Result PresentFrame(const Device& device, const Viewport& viewport) {return (PresentFrame(viewport.GetSwapchain(), viewport.GetSemaphores(), device.GetQueue(QueueType::Present)));}

			//Result RecordCommands(const VkSwapchainKHR& swapchain, const std::vector<VkImage>& swapchainImages, const std::vector<VkImageView>& swapchainViews, const VkExtent2D& swapchainExtent, const std::vector<VkSemaphore>& canPresentSemaphores, const VkQueue& graphicsQueue, const VkQueue& presentQueue);
			//Result RecordCommands(const Viewport& viewport, const VkQueue& graphicsQueue, const VkQueue& presentQueue)
			//	{return (RecordCommands(viewport.GetSwapchain(), viewport.GetImages(), viewport.GetViews(), viewport.GetExtent(), viewport.GetSemaphores(), graphicsQueue, presentQueue));}
			//Result RecordCommands(const Viewport& viewport, const Device& device)
			//	{return (RecordCommands(viewport.GetSwapchain(), viewport.GetImages(), viewport.GetViews(), viewport.GetExtent(), viewport.GetSemaphores(), device.GetQueue(QueueType::Graphics), device.GetQueue(QueueType::Present)));}
	};
}
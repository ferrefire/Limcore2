#pragma once

#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "error.hpp"
#include "command.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

namespace Limcore
{
	#define FRAME_FENCE_TIMEOUT 1000000000
	#define ACQUIRE_IMAGE_TIMEOUT 1000000000
	#define NO_PRESENT_IMAGE UINT32_MAX

	enum class RendererState {Waited, Began, Ended, Submitted, Presented};

	enum class AttachmentIndexType {PresentIndex, FrameIndex};

	struct RendererConfig
	{
		uint32_t maxFramesInFlight = 2;
		std::function<void()> swapchainRecreateCallback;
		bool log = false;
	};

	struct RendererAttachment
	{
		const std::vector<VkImageView>* views;
		AttachmentIndexType indexType = AttachmentIndexType::FrameIndex;
		VkImageLayout layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		VkAttachmentLoadOp loadOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp storeOperation = VK_ATTACHMENT_STORE_OP_STORE;
		VkClearValue clearValue = {{1.0f, 1.0f, 1.0f, 1.0f}};
	};

	struct RendererPass
	{
		std::vector<RendererAttachment> colorAttachments;
		RendererAttachment depthAttachment;
		RendererAttachment stencilAttachment;
		std::vector<std::function<void(const VkCommandBuffer&, const uint32_t&, const uint32_t&)>> preRenderingCalls;
		std::vector<std::function<void(const VkCommandBuffer&, const uint32_t&, const uint32_t&)>> renderingCalls;
		std::vector<std::function<void(const VkCommandBuffer&, const uint32_t&, const uint32_t&)>> postRenderingCalls;
		const VkExtent2D* extent = nullptr;
		bool useDepth = false;
		bool useStencil = false;

		void RegisterPreRenderingCall(std::function<void(const VkCommandBuffer&, const uint32_t&, const uint32_t&)> call)
			{preRenderingCalls.push_back(call);}
		template <class T> void RegisterPreRenderingCall(T* object, void(T::*call)(const VkCommandBuffer&, const uint32_t&, const uint32_t&))
			{RegisterPreRenderingCall(std::bind_front(call, object));}

		void RegisterRenderingCall(std::function<void(const VkCommandBuffer&, const uint32_t&, const uint32_t&)> call)
			{renderingCalls.push_back(call);}
		template <class T> void RegisterRenderingCall(T* object, void(T::*call)(const VkCommandBuffer&, const uint32_t&, const uint32_t&))
			{RegisterRenderingCall(std::bind_front(call, object));}

		void RegisterPostRenderingCall(std::function<void(const VkCommandBuffer&, const uint32_t&, const uint32_t&)> call)
			{postRenderingCalls.push_back(call);}
		template <class T> void RegisterPostRenderingCall(T* object, void(T::*call)(const VkCommandBuffer&, const uint32_t&, const uint32_t&))
			{RegisterPostRenderingCall(std::bind_front(call, object));}
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
			std::vector<RendererPass> rendererPasses;

			RendererState state = RendererState::Presented;
			uint32_t frameIndex = 0;
			uint32_t presentIndex = NO_PRESENT_IMAGE;
			bool swapchainOutOfDate = false;

			[[nodiscard]] Result AllocateCommandBuffers();

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
			[[nodiscard]] const bool& SwapchainOutOfDate() const noexcept {return (swapchainOutOfDate);}

			void AddRendererPass(const RendererPass& rendererPass) {rendererPasses.push_back(rendererPass);}
			[[nodiscard]] Result CreateRenderingAttachment(VkRenderingAttachmentInfo& createdAttachment, const RendererAttachment& rendererAttachment);
			[[nodiscard]] Result CreateRenderingAttachments(std::vector<VkRenderingAttachmentInfo>& createdAttachments, const std::vector<RendererAttachment>& rendererAttachments);

			Result WaitForFrame();
			Result BeginFrame(const VkSwapchainKHR& swapchain);
			Result BeginFrame(const Viewport& viewport) {return (BeginFrame(viewport.GetSwapchain()));}
			Result RenderFrame();
			Result EndFrame(const VkQueue& graphicsQueue, const std::vector<VkSemaphore>& canPresentSemaphores, std::vector<VkSemaphoreSubmitInfo> waitInfos, std::vector<VkSemaphoreSubmitInfo> signalInfos);
			Result EndFrame(const Device& device, const Viewport& viewport) {return (EndFrame(device.GetQueue(QueueType::Graphics), viewport.GetSemaphores(), {}, {}));}
			Result PresentFrame(const VkSwapchainKHR& swapchain, const std::vector<VkSemaphore>& canPresentSemaphores, const VkQueue& presentQueue);
			Result PresentFrame(const Device& device, const Viewport& viewport) {return (PresentFrame(viewport.GetSwapchain(), viewport.GetSemaphores(), device.GetQueue(QueueType::Present)));}
	};
}
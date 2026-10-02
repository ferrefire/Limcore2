#pragma once

#include "error.hpp"
#include "window.hpp"
#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Limcore
{
	class Viewport
	{
		private:
			VkDevice logicalDevice = nullptr;

			VkSwapchainKHR swapchain = nullptr;
			VkExtent2D extent{};
			std::vector<VkImage> images;
			std::vector<VkImageView> views;
			std::vector<VkSemaphore> canPresentSemaphores;

			bool log = false;

			[[nodiscard]] Result CreateSwapchain(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const WindowConfig& windowConfig);
			[[nodiscard]] Result RetrieveImages();
			[[nodiscard]] Result CreateViews(const WindowConfig& windowConfig);
			[[nodiscard]] Result TransitionLayouts(const uint32_t& queueFamilyIndex, const VkQueue& graphicsQueue);

		public:
			Viewport() noexcept = default;
			~Viewport() noexcept {Destroy();}

			Viewport(const Viewport&) = delete;
			Viewport& operator=(const Viewport&) = delete;

			//Todo: implement move operators.

			[[nodiscard]] Result Create(const VkDevice& logicalDevice, const VkPhysicalDevice& physicalDevice, const uint32_t& queueFamilyIndex, const VkQueue& graphicsQueue, const VkSurfaceKHR& surface, const WindowConfig& windowConfig, bool log = false);
			[[nodiscard]] Result Create(const Device& device, const Window& window, bool log = false)
				{return (Create(device.GetLogicalDevice(), device.GetPhysicalDevice(), device.GetSelectedQueueFamily(), device.GetQueue(QueueType::Graphics), window.GetSurface(), window.GetConfig(), log));}
			
			void Destroy() noexcept;

			[[nodiscard]] bool IsValid() const noexcept;
			[[nodiscard]] const VkSwapchainKHR& GetSwapchain() const noexcept {return (swapchain);}
			[[nodiscard]] const VkExtent2D& GetExtent() const noexcept {return (extent);}
			[[nodiscard]] const VkImage& GetImage(uint32_t index) const {return (images[index]);}
			[[nodiscard]] const std::vector<VkImage>& GetImages() const noexcept {return (images);}
			[[nodiscard]] const VkImageView& GetView(uint32_t index) const {return (views[index]);}
			[[nodiscard]] const std::vector<VkImageView>& GetViews() const noexcept {return (views);}
			[[nodiscard]] const VkSemaphore& GetSemaphore(uint32_t index) const {return (canPresentSemaphores[index]);}
			[[nodiscard]] const std::vector<VkSemaphore>& GetSemaphores() const noexcept {return (canPresentSemaphores);}

			void TransitionImageToColor(const uint32_t& index, const VkCommandBuffer& commandBuffer);
			//void TransitionImageToColor(const Renderer& renderer) {TransitionImageToColor(renderer.GetPresentIndex(), renderer.GetCommandBuffer());}
			void TransitionImageToPresent(const uint32_t& index, const VkCommandBuffer& commandBuffer);
			//void TransitionImageToPresent(const Renderer& renderer) {TransitionImageToPresent(renderer.GetPresentIndex(), renderer.GetCommandBuffer());}
	};
}
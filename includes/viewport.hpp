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
			[[nodiscard]] Result CreateSemaphores();

		public:
			Viewport() noexcept = default;
			~Viewport() noexcept {Destroy();}

			Viewport(const Viewport&) = delete;
			Viewport& operator=(const Viewport&) = delete;

			//Todo: implement move operators.

			[[nodiscard]] Result Create(const VkDevice& logicalDevice, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const WindowConfig& windowConfig, bool log = false);
			[[nodiscard]] Result Create(const Device& device, const Window& window, bool log = false)
				{return (Create(device.GetLogicalDevice(), device.GetPhysicalDevice(), window.GetSurface(), window.GetConfig(), log));}
			
			void Destroy() noexcept;

			[[nodiscard]] bool IsValid() const noexcept;
			[[nodiscard]] const VkSwapchainKHR& GetSwapchain() const noexcept {return (swapchain);}
			[[nodiscard]] const VkExtent2D& GetExtent() const noexcept {return (extent);}
			[[nodiscard]] const std::vector<VkImage>& GetImages() const noexcept {return (images);}
			[[nodiscard]] const std::vector<VkImageView>& GetViews() const noexcept {return (views);}
			[[nodiscard]] const std::vector<VkSemaphore>& GetSemaphores() const noexcept {return (canPresentSemaphores);}
	};
}
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
			std::vector<VkImage> images;
			std::vector<VkImageView> views;
			std::vector<VkSemaphore> presentSemaphores;
			bool log = false;

			[[nodiscard]] Result<void> CreateSwapchain(const Device& device, const Window& window);
			[[nodiscard]] Result<void> RetrieveImages();
			[[nodiscard]] Result<void> CreateViews(const Window& window);
			[[nodiscard]] Result<void> CreateSemaphores();

		public:
			Viewport() noexcept = default;
			~Viewport() noexcept {Destroy();}

			Viewport(const Viewport&) = delete;
			Viewport& operator=(const Viewport&) = delete;

			//Todo: implement move operators.

			[[nodiscard]] Result<void> Create(const Device& device, const Window& window, bool log = false);

			void Destroy() noexcept;

			[[nodiscard]] bool IsValid() const noexcept;
			[[nodiscard]] const VkSwapchainKHR& GetSwapchain() const noexcept {return (swapchain);}
	};
}
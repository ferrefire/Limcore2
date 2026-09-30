#pragma once

#include "error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

namespace Limcore
{
	#define DEFAULT_PRESENT_MODE VK_PRESENT_MODE_FIFO_KHR
	#define DEFAULT_SURFACE_FORMAT VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}

	enum class WindowMode {Windowed, Fullscreen, Borderless};

	//Todo: seperate window size and framebuffer size.
	struct WindowConfig
	{
		GLFWmonitor* monitor = nullptr;
		WindowMode mode = WindowMode::Windowed;
		VkPresentModeKHR presentMode = DEFAULT_PRESENT_MODE;
		VkSurfaceFormatKHR surfaceFormat = DEFAULT_SURFACE_FORMAT;
		uint32_t width = 0;
		uint32_t height = 0;
		bool log = false;
	};
	
	class Window
	{
		private:
			VkInstance instance = nullptr;
			GLFWwindow* windowData = nullptr;
			VkSurfaceKHR surface = nullptr;

			WindowConfig config{};
			

			[[nodiscard]] Result<void> CreateFrame();
			[[nodiscard]] Result<void> CreateSurface(const VkPhysicalDevice& physicalDevice);
			[[nodiscard]] Result<void> SelectPresentMode(const VkPhysicalDevice& physicalDevice);
			[[nodiscard]] Result<void> SelectSurfaceFormat(const VkPhysicalDevice& physicalDevice);

		public:
			Window() noexcept = default;
			~Window() noexcept {Destroy();}

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			//Todo: implement move operators.

			[[nodiscard]] Result<void> Create(const VkInstance& vulkanInstance, const VkPhysicalDevice& physicalDevice, WindowConfig windowConfig);

			void Destroy() noexcept;

			[[nodiscard]] bool IsValid() const noexcept;
			[[nodiscard]] const VkSurfaceKHR& GetSurface() const noexcept {return (surface);}
			[[nodiscard]] const WindowConfig& GetConfig() const noexcept {return (config);}

			bool ShouldClose() const noexcept;
	};

	std::ostream& operator<<(std::ostream& out, const Window& window);
}
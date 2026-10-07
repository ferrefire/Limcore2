#pragma once

#include "error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <utility>

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
		uint32_t windowWidth = 0;
		uint32_t windowHeight = 0;
		uint32_t viewportWidth = 0;
		uint32_t viewportHeight = 0;
		bool log = false;
	};
	
	class Window
	{
		private:
			VkInstance instance = nullptr;
			GLFWwindow* windowData = nullptr;
			VkSurfaceKHR surface = nullptr;

			WindowConfig config{};
			

			[[nodiscard]] Result CreateFrame();
			[[nodiscard]] Result CreateSurface(const VkPhysicalDevice& physicalDevice);
			[[nodiscard]] Result SelectPresentMode(const VkPhysicalDevice& physicalDevice);
			[[nodiscard]] Result SelectSurfaceFormat(const VkPhysicalDevice& physicalDevice);

		public:
			Window() noexcept = default;
			~Window() noexcept {Destroy();}

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			Window(Window&& other) noexcept :
				instance(std::exchange(other.instance, nullptr)),
				windowData(std::exchange(other.windowData, nullptr)),
				surface(std::exchange(other.surface, nullptr)),
				config(std::exchange(other.config, {})) {}
			
			Window& operator=(Window&& other) noexcept
			{
				if (this != &other)
				{
					Destroy();
					instance = std::exchange(other.instance, nullptr);
					windowData = std::exchange(other.windowData, nullptr);
					surface = std::exchange(other.surface, nullptr);
					config = std::exchange(other.config, {});
				}
				return (*this);
			}

			[[nodiscard]] Result Create(const VkInstance& vulkanInstance, const VkPhysicalDevice& physicalDevice, WindowConfig windowConfig);

			void Destroy() noexcept;

			[[nodiscard]] bool IsValid() const noexcept;
			[[nodiscard]] const VkSurfaceKHR& GetSurface() const noexcept {return (surface);}
			[[nodiscard]] const WindowConfig& GetConfig() const noexcept {return (config);}

			bool ShouldClose() const noexcept;

			void Resized();
	};

	std::ostream& operator<<(std::ostream& out, const Window& window);
}
#pragma once

#include "error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

namespace Limcore
{
	enum class WindowMode {Windowed, Fullscreen, Borderless};

	struct WindowConfig
	{
		GLFWmonitor* monitor = nullptr;
		WindowMode mode = WindowMode::Windowed;
		uint32_t width = 0;
		uint32_t height = 0;
	};
	
	class Window
	{
		private:
			GLFWwindow* windowData = nullptr;

			[[nodiscard]] Result<void> CreateFrame(WindowConfig& config);

		public:
			Window() noexcept = default;
			~Window() noexcept {Destroy();}

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			//Implement move operators.

			[[nodiscard]] Result<void> Create(WindowConfig config, const VkPhysicalDevice& physicalDevice);

			void Destroy() noexcept;

			bool ShouldClose();
	};
}
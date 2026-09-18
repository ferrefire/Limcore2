#include "window.hpp"

#include <cassert>

namespace Limcore
{
	Result<void> Window::Create(const VkInstance& vulkanInstance, const VkPhysicalDevice& physicalDevice, WindowConfig config)
	{
		assert(vulkanInstance != nullptr);
		assert(physicalDevice != nullptr);

		instance = vulkanInstance;

		Result<void> frameCreation = CreateFrame(config);
		if (!frameCreation) {return (std::unexpected(Error(frameCreation.error(), "Failed to create window")));}

		Result<void> surfaceCreation = CreateSurface(physicalDevice, config);
		if (!surfaceCreation) {return (std::unexpected(Error(surfaceCreation.error(), "Failed to create window")));}

		return (Result<void>());
	}

	Result<void> Window::CreateFrame(WindowConfig& config)
	{
		assert(windowData == nullptr);

		GLFWmonitor* monitor = config.monitor;
		if (monitor == nullptr) {monitor = glfwGetPrimaryMonitor();}
		if (monitor == nullptr) {return (std::unexpected(Error{ErrorCode::GlfwError, "Failed to find a monitor"}));}
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		if (config.width > mode->width) {config.width = mode->width;}
		if (config.height > mode->height) {config.height = mode->height;}
		if (config.width == 0) {config.width = mode->width / (config.mode == WindowMode::Windowed ? 2 : 1);}
		if (config.height == 0) {config.height = mode->height / (config.mode == WindowMode::Windowed ? 2 : 1);}

		windowData = glfwCreateWindow(config.width, config.height, "Limcore", (config.mode == WindowMode::Fullscreen ? monitor : nullptr), nullptr);
		if (windowData == nullptr) {return (std::unexpected(Error{ErrorCode::GlfwError, "Failed to create window"}));}

		std::cout << "Window frame created" << std::endl;

		return (Result<void>());
	}

	Result<void> Window::CreateSurface(const VkPhysicalDevice& physicalDevice, WindowConfig& config)
	{
		assert(windowData != nullptr);
		assert(surface == nullptr);

		VkResult result = glfwCreateWindowSurface(instance, windowData, nullptr, &surface);
		if (result != VK_SUCCESS || surface == nullptr) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create surface", result}));}

		std::cout << "Window surface created" << std::endl;

		return (Result<void>());
	}

	void Window::Destroy() noexcept
	{
		if (windowData != nullptr)
		{
			glfwDestroyWindow(windowData);
			windowData = nullptr;
			std::cout << "Window frame destroyed" << std::endl;
		}

		if (surface != nullptr)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = nullptr;
			instance = nullptr;
			std::cout << "Window surface destroyed" << std::endl;
		}
	}

	const VkSurfaceKHR& Window::GetSurface() const noexcept
	{
		return (surface);
	}

	bool Window::ShouldClose() const noexcept
	{
		if (windowData == nullptr) {return (true);}

		return (glfwWindowShouldClose(windowData));
	}
}
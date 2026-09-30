#include "window.hpp"

#include "printer.hpp"

#include <cassert>

namespace Limcore
{
	Result<void> Window::Create(const VkInstance& vulkanInstance, const VkPhysicalDevice& physicalDevice, WindowConfig windowConfig)
	{
		assert(vulkanInstance != nullptr);
		assert(physicalDevice != nullptr);

		instance = vulkanInstance;
		config = windowConfig;

		Result<void> frameCreation = CreateFrame();
		if (!frameCreation) {return (std::unexpected(Error(frameCreation.error(), "Failed to create window")));}

		Result<void> surfaceCreation = CreateSurface(physicalDevice);
		if (!surfaceCreation) {return (std::unexpected(Error(surfaceCreation.error(), "Failed to create window")));}

		Result<void> presentModeSelection = SelectPresentMode(physicalDevice);

		Result<void> surfaceFormatSelection = SelectSurfaceFormat(physicalDevice);
		if (!surfaceFormatSelection) {return (std::unexpected(Error(surfaceFormatSelection.error(), "Failed to create window")));}

		return (Result<void>());
	}

	Result<void> Window::CreateFrame()
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

		if (config.log) {std::cout << "Window frame created" << std::endl;}

		return (Result<void>());
	}

	Result<void> Window::CreateSurface(const VkPhysicalDevice& physicalDevice)
	{
		assert(windowData != nullptr);
		assert(surface == nullptr);

		VkResult result = glfwCreateWindowSurface(instance, windowData, nullptr, &surface);
		if (result != VK_SUCCESS || surface == nullptr) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create surface", result}));}

		if (config.log) {std::cout << "Window surface created" << std::endl;}

		return (Result<void>());
	}

	Result<void> Window::SelectPresentMode(const VkPhysicalDevice& physicalDevice)
	{
		assert(physicalDevice != nullptr);
		assert(surface != nullptr);

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, availablePresentModes.data());

		bool presentModeFound = false;
		for (const VkPresentModeKHR& presentMode : availablePresentModes)
		{
			if (presentMode == config.presentMode)
			{
				presentModeFound = true;
				break;
			}
		}

		if (!presentModeFound)
		{
			std::cerr << "Target present mode not found: " << EnumName(config.presentMode) << "." << std::endl;

			if (config.presentMode != DEFAULT_PRESENT_MODE)
			{
				std::cerr << "Falling back to default: VK_PRESENT_MODE_FIFO_KHR." << std::endl;
				config.presentMode = DEFAULT_PRESENT_MODE;

				return (SelectPresentMode(physicalDevice));
			}
			
			std::cerr << "Available present modes: ";
			for (size_t i = 0; i < availablePresentModes.size(); i++) {std::cerr << EnumName(availablePresentModes[i]) << (i + 1 == availablePresentModes.size() ? "" : ", ");}
			std::cerr << std::endl;
				
			return (std::unexpected(Error(ErrorCode::Unknown, "Failed to find valid present mode")));
		}

		if (config.log) {std::cout << "Window present mode selected" << std::endl;}

		return (Result<void>());
	}

	Result<void> Window::SelectSurfaceFormat(const VkPhysicalDevice& physicalDevice)
	{
		assert(physicalDevice != nullptr);
		assert(surface != nullptr);

		uint32_t availableSurfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableSurfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> availableSurfaceFormats(availableSurfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableSurfaceFormatCount, availableSurfaceFormats.data());

		bool surfaceFormatFound = false;
		for (const VkSurfaceFormatKHR& surfaceFormat : availableSurfaceFormats)
		{
			if (surfaceFormat.format == config.surfaceFormat.format && surfaceFormat.colorSpace == config.surfaceFormat.colorSpace)
			{
				surfaceFormatFound = true;
				break;
			}
		}

		if (!surfaceFormatFound)
		{
			std::cerr << "Target surface format not found: " << EnumName(config.surfaceFormat.format) <<
				" " << EnumName(config.surfaceFormat.colorSpace) << "." << std::endl;

			if (config.surfaceFormat.format != DEFAULT_SURFACE_FORMAT.format || config.surfaceFormat.colorSpace != DEFAULT_SURFACE_FORMAT.colorSpace)
			{
				std::cerr << "Falling back to default: VK_FORMAT_B8G8R8A8_SRGB VK_COLOR_SPACE_SRGB_NONLINEAR_KHR." << std::endl;
				config.surfaceFormat = DEFAULT_SURFACE_FORMAT;
				
				return (SelectSurfaceFormat(physicalDevice));
			}

			std::cerr << "Available surface formats: ";
			for (size_t i = 0; i < availableSurfaceFormats.size(); i++)
			{
				std::cerr << EnumName(availableSurfaceFormats[i].format) << " " <<
				EnumName(availableSurfaceFormats[i].colorSpace) << (i + 1 == availableSurfaceFormats.size() ? "" : ", ");
			}
			std::cerr << std::endl;
			
			return (std::unexpected(Error(ErrorCode::Unknown, "Failed to find valid surface format")));
		}

		if (config.log) {std::cout << "Window surface format selected" << std::endl;}

		return (Result<void>());
	}

	void Window::Destroy() noexcept
	{
		if (windowData != nullptr)
		{
			glfwDestroyWindow(windowData);
			windowData = nullptr;
			if (config.log) {std::cout << "Window frame destroyed" << std::endl;}
		}

		if (surface != nullptr)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = nullptr;
			instance = nullptr;
			if (config.log) {std::cout << "Window surface destroyed" << std::endl;}
		}
	}

	bool Window::IsValid() const noexcept
	{
		if (instance == nullptr) {return (false);}
		if (windowData == nullptr) {return (false);}
		if (surface == nullptr) {return (false);}

		return (true);
	}

	bool Window::ShouldClose() const noexcept
	{
		if (windowData == nullptr) {return (true);}

		return (glfwWindowShouldClose(windowData));
	}
}
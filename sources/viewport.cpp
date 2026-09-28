#include "viewport.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

namespace Limcore
{
	Result<void> Viewport::Create(const Device& device, const Window& window, bool log)
	{
		assert(device.GetLogicalDevice() != nullptr);

		this->log = log;
		logicalDevice = device.GetLogicalDevice();

		Result<void> swapchainCreation = CreateSwapchain(device, window);
		if (!swapchainCreation) {return (std::unexpected(Error(swapchainCreation.error(), "Failed to create viewport")));}

		Result<void> imagesRetrieval = RetrieveImages();
		if (!imagesRetrieval) {return (std::unexpected(Error(imagesRetrieval.error(), "Failed to create viewport")));}

		Result<void> viewsCreation = CreateViews(window);
		if (!viewsCreation) {return (std::unexpected(Error(viewsCreation.error(), "Failed to create viewport")));}

		return (Result<void>());
	}

	Result<void> Viewport::CreateSwapchain(const Device& device, const Window& window)
	{
		assert(device.GetPhysicalDevice() != nullptr);
		assert(device.GetLogicalDevice() != nullptr);
		assert(window.GetSurface() != nullptr);

		const WindowConfig& windowConfig = window.GetConfig();

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysicalDevice(), window.GetSurface(), &surfaceCapabilities);

		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {imageCount = surfaceCapabilities.maxImageCount;}

		VkExtent2D extent = surfaceCapabilities.currentExtent;
		if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
		{
			extent.width = std::clamp(windowConfig.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			extent.height = std::clamp(windowConfig.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = window.GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = windowConfig.surfaceFormat.format;
		createInfo.imageColorSpace = windowConfig.surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = surfaceCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = windowConfig.presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = nullptr;

		VkResult result = vkCreateSwapchainKHR(device.GetLogicalDevice(), &createInfo, nullptr, &swapchain);
		if (result != VK_SUCCESS || swapchain == nullptr) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create swapchain", result}));}

		if (log) {std::cout << "Viewport swapchain created" << std::endl;}

		return (Result<void>());
	}

	Result<void> Viewport::RetrieveImages()
	{
		assert(swapchain != nullptr);
		assert(logicalDevice != nullptr);
		assert(images.empty());

		uint32_t imageCount = 0;
		VkResult result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
		if (result != VK_SUCCESS || imageCount == 0) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to retrieve images", result}));}
		images.resize(imageCount);
		result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, images.data());
		if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to retrieve images", result}));}

		if (log) {std::cout << "Viewport images retrieved" << std::endl;}

		return (Result<void>());
	}

	Result<void> Viewport::CreateViews(const Window& window)
	{
		assert(logicalDevice != nullptr);
		assert(!images.empty());
		assert(views.empty());

		views.resize(images.size());

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = window.GetConfig().surfaceFormat.format;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		for (size_t i = 0; i < images.size(); i++)
		{
			createInfo.image = images[i];

			VkResult result = vkCreateImageView(logicalDevice, &createInfo, nullptr, &views[i]);
			if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create views", result}));}
		}

		if (log) {std::cout << "Viewport views created" << std::endl;}

		return (Result<void>());
	}

	void Viewport::Destroy() noexcept
	{
		if (logicalDevice == nullptr) {return;}

		if (!views.empty())
		{
			for (VkImageView& view : views) {vkDestroyImageView(logicalDevice, view, nullptr);}
			views.clear();
			if (log) {std::cout << "Viewport views destroyed" << std::endl;}
		}

		if (swapchain)
		{
			vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
			swapchain = nullptr;
			if (log) {std::cout << "Viewport swapchain destroyed" << std::endl;}
		}

		if (!images.empty()) {images.clear();}

		logicalDevice = nullptr;
	}
}
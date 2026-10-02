#include "viewport.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

namespace Limcore
{
	Result Viewport::Create(const VkDevice& logicalDevice, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const WindowConfig& windowConfig, bool log)
	{
		assert(logicalDevice != nullptr);
		assert(physicalDevice != nullptr);
		assert(surface != nullptr);

		this->log = log;
		this->logicalDevice = logicalDevice;

		Result swapchainCreation = CreateSwapchain(physicalDevice, surface, windowConfig);
		if (!swapchainCreation) {return (std::unexpected(Error(swapchainCreation.error(), "Failed to create viewport")));}

		Result imagesRetrieval = RetrieveImages();
		if (!imagesRetrieval) {return (std::unexpected(Error(imagesRetrieval.error(), "Failed to create viewport")));}

		Result viewsCreation = CreateViews(windowConfig);
		if (!viewsCreation) {return (std::unexpected(Error(viewsCreation.error(), "Failed to create viewport")));}

		Result semaphoresCreation = CreateSemaphores();
		if (!semaphoresCreation) {return (std::unexpected(Error(semaphoresCreation.error(), "Failed to create viewport")));}

		return (Result());
	}

	Result Viewport::CreateSwapchain(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const WindowConfig& windowConfig)
	{
		assert(physicalDevice != nullptr);
		assert(surface != nullptr);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {imageCount = surfaceCapabilities.maxImageCount;}

		VkExtent2D windowExtent = surfaceCapabilities.currentExtent;
		if (windowExtent.width == UINT32_MAX || windowExtent.height == UINT32_MAX)
		{
			windowExtent.width = std::clamp(windowConfig.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			windowExtent.height = std::clamp(windowConfig.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}
		extent = windowExtent;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
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

		VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchain);
		if (result != VK_SUCCESS || swapchain == nullptr) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create swapchain", result}));}

		if (log) {std::cout << "Viewport swapchain created" << std::endl;}

		return (Result());
	}

	Result Viewport::RetrieveImages()
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

		return (Result());
	}

	Result Viewport::CreateViews(const WindowConfig& windowConfig)
	{
		assert(logicalDevice != nullptr);
		assert(!images.empty());
		assert(views.empty());

		views.resize(images.size());

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = windowConfig.surfaceFormat.format;
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

		return (Result());
	}

	Result Viewport::CreateSemaphores()
	{
		assert(logicalDevice != nullptr);
		assert(canPresentSemaphores.empty());

		canPresentSemaphores.resize(images.size());

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (size_t i = 0; i < canPresentSemaphores.size(); i++)
		{
			VkResult result = vkCreateSemaphore(logicalDevice, &createInfo, nullptr, &canPresentSemaphores[i]);
			if (result != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, "Failed to create semaphores", result}));}
		}

		if (log) {std::cout << "Viewport semaphores created" << std::endl;}

		return (Result());
	}

	void Viewport::Destroy() noexcept
	{
		if (logicalDevice == nullptr) {return;}

		vkDeviceWaitIdle(logicalDevice);

		if (!canPresentSemaphores.empty())
		{
			for (VkSemaphore& semaphore : canPresentSemaphores) {vkDestroySemaphore(logicalDevice, semaphore, nullptr);}
			canPresentSemaphores.clear();
			if (log) {std::cout << "Viewport semaphores destroyed" << std::endl;}
		}

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

	bool Viewport::IsValid() const noexcept
	{
		if (logicalDevice == nullptr) {return (false);}
		if (swapchain == nullptr) {return (false);}
		if (images.empty()) {return (false);}
		if (views.empty()) {return (false);}

		return (true);
	}
}
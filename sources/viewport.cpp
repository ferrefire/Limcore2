#include "viewport.hpp"

#include "command.hpp"
#include "utility.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>

namespace Limcore
{
	Result Viewport::Create(const VkDevice& logicalDevice, const VkPhysicalDevice& physicalDevice, const uint32_t& queueFamilyIndex, const VkQueue& graphicsQueue, const VkSurfaceKHR& surface, const WindowConfig& windowConfig, bool log)
	{
		assert(logicalDevice != nullptr);
		assert(physicalDevice != nullptr);
		assert(queueFamilyIndex != NO_QUEUE_FAMILY);
		assert(graphicsQueue != nullptr);
		assert(surface != nullptr);

		const std::string message = "Failed to create viewport";

		this->log = log;
		this->logicalDevice = logicalDevice;

		Result result = CreateSwapchain(physicalDevice, surface, windowConfig);
		RETURN_ERROR(result, message)

		result = RetrieveImages();
		RETURN_ERROR(result, message)

		result = CreateViews(windowConfig);
		RETURN_ERROR(result, message)

		result = CreateSemaphores(canPresentSemaphores, logicalDevice, images.size());
		RETURN_ERROR(result, message)

		result = TransitionLayouts(queueFamilyIndex, graphicsQueue);
		RETURN_ERROR(result, message)

		return (Result());
	}

	Result Viewport::Recreate(const VkDevice& logicalDevice, const VkPhysicalDevice& physicalDevice, const uint32_t& queueFamilyIndex, const VkQueue& graphicsQueue, const VkSurfaceKHR& surface, const WindowConfig& windowConfig)
	{
		if (log) {std::cout << "Recreating viewport" << std::endl;}

		oldSwapchain = swapchain;
		swapchain = nullptr;

		Destroy();

		Result result = Create(logicalDevice, physicalDevice, queueFamilyIndex, graphicsQueue, surface, windowConfig, false);
		RETURN_ERROR(result, "Failed to recreate swapchain")

		if (oldSwapchain != nullptr)
		{
			vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
			oldSwapchain = nullptr;
		}

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

		//VkExtent2D windowExtent = surfaceCapabilities.currentExtent;
		//if (windowExtent.width == UINT32_MAX || windowExtent.height == UINT32_MAX)
		//{
		//	windowExtent.width = std::clamp(windowConfig.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		//	windowExtent.height = std::clamp(windowConfig.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		//}

		VkExtent2D viewportExtent{};
		viewportExtent.width = std::clamp(windowConfig.viewportWidth, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		viewportExtent.height = std::clamp(windowConfig.viewportHeight, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		extent = viewportExtent;

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
		createInfo.oldSwapchain = oldSwapchain;

		VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchain);
		RETURN_VK_ERROR(result, "Failed to create swapchain")

		if (log) {std::cout << "Viewport swapchain created" << std::endl;}

		return (Result());
	}

	Result Viewport::RetrieveImages()
	{
		assert(swapchain != nullptr);
		assert(logicalDevice != nullptr);
		assert(images.empty());

		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		VkResult result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, images.data());
		RETURN_VK_ERROR(result, "Failed to retrieve images")

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
			RETURN_VK_ERROR(result, "Failed to create views")
		}

		if (log) {std::cout << "Viewport views created" << std::endl;}

		return (Result());
	}

	Result Viewport::TransitionLayouts(const uint32_t& queueFamilyIndex, const VkQueue& graphicsQueue)
	{
		const std::string message = "Failed to transition layouts";

		VkCommandPool commandPool = nullptr;
		VkCommandBuffer commandBuffer = nullptr;
		VkFence fence = nullptr;

		Result result = CreateCommandPool(commandPool, logicalDevice, queueFamilyIndex);
		RETURN_ERROR(result, message)

		result = AllocateCommandBuffer(commandBuffer, commandPool, logicalDevice);
		RETURN_ERROR(result, message)

		result = CreateFence(fence, logicalDevice, 0);
		RETURN_ERROR(result, message)

		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier.srcAccessMask = 0;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		std::vector<VkImageMemoryBarrier2> barriers(images.size());

		for (size_t i = 0; i < barriers.size(); i++)
		{
			barriers[i] = barrier;
			barriers[i].image = images[i];
		}

		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.imageMemoryBarrierCount = CUI(barriers.size());
		dependency.pImageMemoryBarriers = barriers.data();

		result = BeginCommand(commandBuffer);
		RETURN_ERROR(result, message)

		vkCmdPipelineBarrier2(commandBuffer, &dependency);

		result = EndCommand(commandBuffer);
		RETURN_ERROR(result, message)

		result = SubmitCommand(commandBuffer, graphicsQueue, {}, {}, fence);
		RETURN_ERROR(result, message)

		result = WaitForFence(fence, logicalDevice);
		RETURN_ERROR(result, message)

		vkDeviceWaitIdle(logicalDevice);
		vkDestroyFence(logicalDevice, fence, nullptr);
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

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

	void Viewport::TransitionImageToColor(const uint32_t& index, const VkCommandBuffer& commandBuffer)
	{
		assert(index < images.size());
		assert(commandBuffer != nullptr);

		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier.srcAccessMask = 0;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = images[index];
		barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.imageMemoryBarrierCount = 1;
		dependency.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(commandBuffer, &dependency);
	}

	void Viewport::TransitionImageToPresent(const uint32_t& index, const VkCommandBuffer& commandBuffer)
	{
		assert(index < images.size());
		assert(commandBuffer != nullptr);

		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = images[index];
		barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.imageMemoryBarrierCount = 1;
		dependency.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(commandBuffer, &dependency);
	}
}
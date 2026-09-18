#include "device.hpp"

#include "utility.hpp"
#include "printer.hpp"

#include <cassert>
#include <exception>

namespace Limcore
{
	Result<void> Device::Create(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, DeviceFeatures features) noexcept
	{
		assert(instance != nullptr);
		assert(physicalDevice != nullptr);
		assert(surface != nullptr);
		assert(this->physicalDevice == nullptr);
		assert(this->logicalDevice == nullptr);

		this->physicalDevice = physicalDevice;

		Result<void> logicalDeviceCreation = CreateLogical(surface, features);
		if (!logicalDeviceCreation) {return (std::unexpected(Error(logicalDeviceCreation.error(), "Failed to create device")));}

		return (Result<void>());
	}

	Result<void> Device::CreateLogical(const VkSurfaceKHR& surface, DeviceFeatures features)
	{
		assert(physicalDevice != nullptr);
		assert(logicalDevice == nullptr);
		assert(surface != nullptr);

		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueFamilyProperties.data());
		int queueFamilyIndex = -1;

		for (size_t i = 0; i < queueCount; i++)
		{
			if (HasFlag(queueFamilyProperties[i].queueFlags, VK_QUEUE_GRAPHICS_BIT) &&
				HasFlag(queueFamilyProperties[i].queueFlags, VK_QUEUE_TRANSFER_BIT) &&
				HasFlag(queueFamilyProperties[i].queueFlags, VK_QUEUE_COMPUTE_BIT) &&
				queueFamilyProperties[i].queueCount >= 4 &&
				(queueFamilyIndex == -1 || queueFamilyProperties[queueFamilyIndex].queueCount < queueFamilyProperties[i].queueCount))
			{
				VkBool32 canPresent = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &canPresent);
				if (canPresent) {queueFamilyIndex = i;}
			}
		}

		if (queueFamilyIndex == -1) {return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to find a valid queue family")));}

		std::cout << "Queue family selected" << std::endl;

		std::vector<float> queuePriorities{1.0f, 1.0f, 1.0f, 1.0f};
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 4;
		queueCreateInfo.pQueuePriorities = queuePriorities.data();
		queueCreateInfos.push_back(queueCreateInfo);

		VkPhysicalDeviceFeatures deviceFeaturesBase{};
		if (features.tesselation) {deviceFeaturesBase.tessellationShader = VK_TRUE;}
		if (features.anisotropic) {deviceFeaturesBase.samplerAnisotropy = VK_TRUE;}
		if (features.shaderDouble) {deviceFeaturesBase.shaderFloat64 = VK_TRUE;}
		if (features.geometryShader) {deviceFeaturesBase.geometryShader = VK_TRUE;}
		if (features.fillModeNonSolid) {deviceFeaturesBase.fillModeNonSolid = VK_TRUE;}
		if (features.depthBounds) {deviceFeaturesBase.depthBounds = VK_TRUE;}
		if (features.compressionBC) {deviceFeaturesBase.textureCompressionBC = VK_TRUE;}
		if (features.multiDrawIndirect) {deviceFeaturesBase.multiDrawIndirect = VK_TRUE;}

		VkPhysicalDeviceVulkan13Features deviceFeatures3{};
		deviceFeatures3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		if (features.synchronization2) {deviceFeatures3.synchronization2 = VK_TRUE;}

		VkPhysicalDeviceVulkan12Features deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		deviceFeatures2.pNext = &deviceFeatures3;
		if (features.nonUniformIndexingShaderSampledImageArray) {deviceFeatures2.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;}

		VkPhysicalDeviceVulkan11Features deviceFeatures1{};
		deviceFeatures1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		deviceFeatures1.pNext = &deviceFeatures2;
		deviceFeatures1.shaderDrawParameters = VK_TRUE;

		VkPhysicalDeviceFeatures2 deviceFeatures{};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.features = deviceFeaturesBase;
		deviceFeatures.pNext = &deviceFeatures1;

		std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = CUI(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = nullptr;
		createInfo.enabledExtensionCount = CUI(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.pNext = &deviceFeatures;

		VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
		if (result != VK_SUCCESS || logicalDevice == nullptr) {return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to create logical device", result)));}

		std::cout << "Logical device created" << std::endl;

		return (Result<void>());
	}

	void Device::Destroy() noexcept
	{
		if (physicalDevice != nullptr)
		{
			physicalDevice = nullptr;
		}

		if (logicalDevice != nullptr)
		{
			vkDestroyDevice(logicalDevice, nullptr);
			logicalDevice = nullptr;
			std::cout << "Logical device destroyed" << std::endl;
		}
	}

	std::vector<DeviceInfo> GetAvailableDevices(const VkInstance& instance)
	{
		assert(instance != nullptr);

		std::vector<DeviceInfo> availableDevices;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {return (availableDevices);}

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

		availableDevices.resize(deviceCount);
		for (size_t i = 0; i < deviceCount; i++)
		{
			availableDevices[i].physicalDevice = physicalDevices[i];
			vkGetPhysicalDeviceProperties(availableDevices[i].physicalDevice, &availableDevices[i].properties);
			availableDevices[i].features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			availableDevices[i].features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			availableDevices[i].features3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			availableDevices[i].features.pNext = &availableDevices[i].features2;
			availableDevices[i].features2.pNext = &availableDevices[i].features3;
			vkGetPhysicalDeviceFeatures2(availableDevices[i].physicalDevice, &availableDevices[i].features);
			availableDevices[i].type = static_cast<DeviceType>(availableDevices[i].properties.deviceType);
		}

		return (availableDevices);
	}

	Result<DeviceInfo> GetDevice(const VkInstance& instance, DeviceType type, DeviceFeatures features)
	{
		assert(instance != nullptr);

		std::vector<DeviceInfo> availableDevices = GetAvailableDevices(instance);

		for (size_t i = 0; i < availableDevices.size(); i++)
		{
			if (availableDevices[i].type != type) {continue;}
			VkPhysicalDeviceFeatures features1 = availableDevices[i].features.features;
			VkPhysicalDeviceVulkan12Features features2 = availableDevices[i].features2;
			VkPhysicalDeviceVulkan13Features features3 = availableDevices[i].features3;
			if (features.tesselation && !features1.tessellationShader) {continue;}
			if (features.anisotropic && !features1.samplerAnisotropy) {continue;}
			if (features.shaderDouble && !features1.shaderFloat64) {continue;}
			if (features.geometryShader && !features1.geometryShader) {continue;}
			if (features.depthBounds && !features1.depthBounds) {continue;}
			if (features.compressionBC && !features1.textureCompressionBC) {continue;}
			if (features.multiDrawIndirect && !features1.multiDrawIndirect) {continue;}
			if (features.nonUniformIndexingShaderSampledImageArray && !features2.shaderSampledImageArrayNonUniformIndexing) {continue;}
			if (features.synchronization2 && !features3.synchronization2) {continue;}

			return (availableDevices[i]);
		}

		Error error{ErrorCode::VulkanError, "Failed to find specified device."};
		return (std::unexpected(error));
	}

	std::ostream& operator<<(std::ostream& out, const DeviceInfo& deviceInfo)
	{
		const VkPhysicalDeviceProperties& properties = deviceInfo.properties;

		out << VAR_VAL(properties.deviceName) << std::endl;
		out << VAR_VAL(properties.vendorID) << std::endl;
		out << ENUM_VAL(properties.deviceType) << std::endl;
		out << VAR_VAL(properties.driverVersion) << std::endl;
		out << VAR_VAL(properties.apiVersion) << " "
			<< VK_API_VERSION_MAJOR(properties.apiVersion) << "."
			<< VK_API_VERSION_MINOR(properties.apiVersion) << "."
			<< VK_API_VERSION_PATCH(properties.apiVersion) << std::endl;

		return (out);
	}
}

std::ostream& operator<<(std::ostream& out, const VkQueueFamilyProperties& queueFamilyProperties)
{
	out << "Queue count: " << queueFamilyProperties.queueCount << std::endl;
	out << "Graphics capable: " << (Limcore::HasFlag(queueFamilyProperties.queueFlags, VK_QUEUE_GRAPHICS_BIT) ? "true" : "false") << std::endl;
	out << "Transfer capable: " << (Limcore::HasFlag(queueFamilyProperties.queueFlags, VK_QUEUE_TRANSFER_BIT) ? "true" : "false") << std::endl;
	out << "Compute capable: " << (Limcore::HasFlag(queueFamilyProperties.queueFlags, VK_QUEUE_COMPUTE_BIT) ? "true" : "false") << std::endl;

	return (out);
}
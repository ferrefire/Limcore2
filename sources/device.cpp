#include "device.hpp"

#include "utility.hpp"
#include "printer.hpp"

#include <cassert>
#include <exception>

namespace Limcore
{
	Result Device::Create(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, DeviceFeatures features, bool log) noexcept
	{
		assert(instance != nullptr);
		assert(physicalDevice != nullptr);
		assert(surface != nullptr);
		assert(this->physicalDevice == nullptr);
		assert(this->logicalDevice == nullptr);

		this->log = log;
		this->physicalDevice = physicalDevice;

		Result queueSelection = SelectQueues(surface);
		if (!queueSelection) {return (std::unexpected(Error(queueSelection.error(), "Failed to create device")));}

		Result logicalDeviceCreation = CreateLogical(features);
		if (!logicalDeviceCreation) {return (std::unexpected(Error(logicalDeviceCreation.error(), "Failed to create device")));}

		Result queueRetrieval = RetrieveQueues();
		if (!queueRetrieval) {return (std::unexpected(Error(queueRetrieval.error(), "Failed to create device")));}

		return (Result());
	}

	Result Device::CreateLogical(DeviceFeatures features)
	{
		assert(physicalDevice != nullptr);
		assert(logicalDevice == nullptr);
		assert(selectedQueueFamilyIndex != NO_QUEUE_FAMILY);

		std::vector<float> queuePriorities{1.0f, 1.0f, 1.0f, 1.0f};
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = selectedQueueFamilyIndex;
		queueCreateInfo.queueCount = (separateQueues ? 4 : 1);
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
		if (features.dynamicRendering) {deviceFeatures3.dynamicRendering = VK_TRUE;}

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

		if (log) {std::cout << "Logical device created" << std::endl;}

		return (Result());
	}

	Result Device::SelectQueues(const VkSurfaceKHR& surface)
	{
		assert(physicalDevice != nullptr);
		assert(surface != nullptr);

		uint32_t queueCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueFamilyProperties.data());
		uint32_t queueFamilyIndex = NO_QUEUE_FAMILY;
		separateQueues = true;

		for (size_t i = 0; i < queueCount; i++)
		{
			if (HasFlag(queueFamilyProperties[i].queueFlags, VK_QUEUE_GRAPHICS_BIT) &&
				HasFlag(queueFamilyProperties[i].queueFlags, VK_QUEUE_TRANSFER_BIT) &&
				HasFlag(queueFamilyProperties[i].queueFlags, VK_QUEUE_COMPUTE_BIT) &&
				(queueFamilyIndex == NO_QUEUE_FAMILY || queueFamilyProperties[queueFamilyIndex].queueCount < queueFamilyProperties[i].queueCount))
			{
				VkBool32 canPresent = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &canPresent);
				if (canPresent) {queueFamilyIndex = i;}
			}
		}

		if (queueFamilyIndex == NO_QUEUE_FAMILY)
		{
			std::cerr << "Available queue families: " << std::endl;
			for (const VkQueueFamilyProperties& properties : queueFamilyProperties) {std::cerr << properties << std::endl;}

			return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to find a valid queue family")));
		}

		if (queueFamilyProperties[queueFamilyIndex].queueCount < 4)
		{
			std::cerr << "Separate queues support not found. Falling back to single queue use." << std::endl;
			separateQueues = false;
		}

		selectedQueueFamilyIndex = queueFamilyIndex;
		if (log) {std::cout << "Queue family selected: " << selectedQueueFamilyIndex << std::endl;}

		return (Result());
	}

	Result Device::RetrieveQueues()
	{
		assert(logicalDevice != nullptr);
		assert(selectedQueueFamilyIndex != NO_QUEUE_FAMILY);

		queues[QueueType::Graphics] = nullptr;
		queues[QueueType::Transfer] = nullptr;
		queues[QueueType::Compute] = nullptr;
		queues[QueueType::Present] = nullptr;

		int queueIndex = -1;
		vkGetDeviceQueue(logicalDevice, selectedQueueFamilyIndex, (separateQueues ? ++queueIndex : 0), &queues[QueueType::Graphics]);
		vkGetDeviceQueue(logicalDevice, selectedQueueFamilyIndex, (separateQueues ? ++queueIndex : 0), &queues[QueueType::Transfer]);
		vkGetDeviceQueue(logicalDevice, selectedQueueFamilyIndex, (separateQueues ? ++queueIndex : 0), &queues[QueueType::Compute]);
		vkGetDeviceQueue(logicalDevice, selectedQueueFamilyIndex, (separateQueues ? ++queueIndex : 0), &queues[QueueType::Present]);

		if (queues[QueueType::Graphics] == nullptr) {return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to retrieve graphics queue")));}
		if (queues[QueueType::Transfer] == nullptr) {return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to retrieve transfer queue")));}
		if (queues[QueueType::Compute] == nullptr) {return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to retrieve compute queue")));}
		if (queues[QueueType::Present] == nullptr) {return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to retrieve present queue")));}

		return (Result());
	}

	void Device::Destroy() noexcept
	{
		if (physicalDevice != nullptr) {physicalDevice = nullptr;}

		if (selectedQueueFamilyIndex != NO_QUEUE_FAMILY) {selectedQueueFamilyIndex = NO_QUEUE_FAMILY;}

		if (!queues.empty()) {queues.clear();}

		if (logicalDevice != nullptr)
		{
			vkDestroyDevice(logicalDevice, nullptr);
			logicalDevice = nullptr;
			if (log) {std::cout << "Logical device destroyed" << std::endl;}
		}
	}

	bool Device::IsValid() const noexcept
	{
		if (physicalDevice == nullptr) {return (false);}
		if (logicalDevice == nullptr) {return (false);}
		if (selectedQueueFamilyIndex == NO_QUEUE_FAMILY) {return (false);}
		if (queues.empty()) {return (false);}

		return (true);
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

	ResultT<DeviceInfo> GetDevice(const VkInstance& instance, DeviceType type, DeviceFeatures features)
	{
		assert(instance != nullptr);

		if (type == DeviceType::Best)
		{
			auto discreteSearch = GetDevice(instance, DeviceType::Discrete, features);
			if (discreteSearch) {return (discreteSearch);}

			auto integratedSearch = GetDevice(instance, DeviceType::Integrated, features);
			if (integratedSearch) {return (integratedSearch);}

			auto virtualSearch = GetDevice(instance, DeviceType::Virtual, features);
			if (virtualSearch) {return (virtualSearch);}

			auto cpuSearch = GetDevice(instance, DeviceType::CPU, features);
			if (cpuSearch) {return (cpuSearch);}

			auto otherSearch = GetDevice(instance, DeviceType::Other, features);
			if (otherSearch) {return (otherSearch);}

			return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to find specified device.")));
		}

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
			if (features.dynamicRendering && !features3.dynamicRendering) {continue;}

			return (availableDevices[i]);
		}

		return (std::unexpected(Error(ErrorCode::VulkanError, "Failed to find specified device.")));
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
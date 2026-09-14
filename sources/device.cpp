#include "device.hpp"

#include <cassert>

namespace Limcore
{
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
			vkGetPhysicalDeviceProperties2(availableDevices[i].physicalDevice, &availableDevices[i].properties);
			availableDevices[i].features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			availableDevices[i].features3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			availableDevices[i].features.pNext = &availableDevices[i].features3;
			vkGetPhysicalDeviceFeatures2(availableDevices[i].physicalDevice, &availableDevices[i].features);
			availableDevices[i].type = static_cast<DeviceType>(availableDevices[i].properties.properties.deviceType);
		}

		return (availableDevices);
	}
}
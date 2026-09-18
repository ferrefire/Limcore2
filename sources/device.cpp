#include "device.hpp"

#include "utility.hpp"
#include "printer.hpp"

#include <cassert>
#include <exception>

namespace Limcore
{
	Device::Device(const VkInstance& instance, DeviceType type, DeviceFeatures features) noexcept
	{

	}

	Result<void> Device::CreatePhysical(const VkInstance& instance, DeviceType type, DeviceFeatures features)
	{
		assert(instance != nullptr);

		return (Result<void>());
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
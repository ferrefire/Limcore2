#pragma once

#include "error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Limcore
{
	enum class DeviceType
	{
		Other = VK_PHYSICAL_DEVICE_TYPE_OTHER,
		Integrated = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
		Discrete = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
		Virtual = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
		CPU = VK_PHYSICAL_DEVICE_TYPE_CPU,
		Best = -1
	};

	struct DeviceInfo
	{
		VkPhysicalDevice physicalDevice = nullptr;
		VkPhysicalDeviceProperties2 properties{};
		VkPhysicalDeviceFeatures2 features{};
		VkPhysicalDeviceVulkan13Features features3{};
		DeviceType type = DeviceType::Other;
	};
	

	class Device
	{
		private:
			VkPhysicalDevice physicalDevice = nullptr;
			VkDevice logicalDevice = nullptr;

		public:
			Device() noexcept = default;
			~Device() noexcept {Destroy();}

			Device(const Device&) = delete;
			Device& operator=(const Device&) = delete;

			//Implement move operators.

			void Destroy() noexcept;
	};

	[[nodiscard]] std::vector<DeviceInfo> GetAvailableDevices(const VkInstance& instance);
}
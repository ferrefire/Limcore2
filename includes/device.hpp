#pragma once

#include "error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

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
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceFeatures2 features{};
		VkPhysicalDeviceVulkan12Features features2{};
		VkPhysicalDeviceVulkan13Features features3{};
		DeviceType type = DeviceType::Other;
	};
	
	struct DeviceFeatures
	{
		bool tesselation = false;
		bool anisotropic = false;
		bool shaderDouble = false;
		bool geometryShader = false;
		bool depthBounds = false;
		bool compressionBC = false;
		bool multiDrawIndirect = false;
		bool nonUniformIndexingShaderSampledImageArray = false;
		bool synchronization2 = false;
		bool fillModeNonSolid = false;
	};

	class Device
	{
		private:
			VkPhysicalDevice physicalDevice = nullptr;
			VkDevice logicalDevice = nullptr;

			[[nodiscard]] Result<void> CreateLogical(const VkSurfaceKHR& surface, DeviceFeatures features);
			[[nodiscard]] Result<void> RetrieveQueues();

		public:
			Device() noexcept = default;
			~Device() noexcept {Destroy();}

			Device(const Device&) = delete;
			Device& operator=(const Device&) = delete;

			//Implement move operators.

			[[nodiscard]] Result<void> Create(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, DeviceFeatures features) noexcept;

			void Destroy() noexcept;
	};

	[[nodiscard]] std::vector<DeviceInfo> GetAvailableDevices(const VkInstance& instance);
	[[nodiscard]] Result<DeviceInfo> GetDevice(const VkInstance& instance, DeviceType type, DeviceFeatures features);

	std::ostream& operator<<(std::ostream& out, const DeviceInfo& deviceInfo);
}

std::ostream& operator<<(std::ostream& out, const VkQueueFamilyProperties& queueFamilyProperties);
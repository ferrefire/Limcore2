#pragma once

#include "error.hpp"
#include "window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <unordered_map>

namespace Limcore
{
	#define NO_QUEUE_FAMILY UINT32_MAX

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
		bool dynamicRendering = false;
	};

	enum class QueueType
	{
		Graphics,
		Transfer,
		Compute,
		Present
	};

	class Device
	{
		private:
			VkPhysicalDevice physicalDevice = nullptr;
			VkDevice logicalDevice = nullptr;
			uint32_t selectedQueueFamilyIndex = NO_QUEUE_FAMILY;
			bool separateQueues = true;
			std::unordered_map<QueueType, VkQueue> queues;
			bool log = false;

			[[nodiscard]] Result CreateLogical(DeviceFeatures features);
			[[nodiscard]] Result SelectQueues(const VkSurfaceKHR& surface); //Todo: add manual selection and better auto selection.
			[[nodiscard]] Result RetrieveQueues();

		public:
			Device() noexcept = default;
			~Device() noexcept {Destroy();}

			Device(const Device&) = delete;
			Device& operator=(const Device&) = delete;

			//Todo: implement move operators.

			[[nodiscard]] Result Create(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, DeviceFeatures features, bool log = false) noexcept;
			[[nodiscard]] Result Create(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const Window& window, DeviceFeatures features, bool log = false) noexcept
				{return (Create(instance, physicalDevice, window.GetSurface(), features, log));}

			void Destroy() noexcept;

			[[nodiscard]] bool IsValid() const noexcept;
			[[nodiscard]] const VkPhysicalDevice& GetPhysicalDevice() const noexcept {return (physicalDevice);}
			[[nodiscard]] const VkDevice& GetLogicalDevice() const noexcept {return (logicalDevice);}
			[[nodiscard]] const uint32_t& GetSelectedQueueFamily() const noexcept {return (selectedQueueFamilyIndex);}
			[[nodiscard]] const VkQueue& GetQueue(QueueType type) const noexcept {return (queues.at(type));}
	};

	[[nodiscard]] std::vector<DeviceInfo> GetAvailableDevices(const VkInstance& instance);
	[[nodiscard]] ResultT<DeviceInfo> GetDevice(const VkInstance& instance, DeviceType type, DeviceFeatures features);

	std::ostream& operator<<(std::ostream& out, const DeviceInfo& deviceInfo);
}

std::ostream& operator<<(std::ostream& out, const VkQueueFamilyProperties& queueFamilyProperties);
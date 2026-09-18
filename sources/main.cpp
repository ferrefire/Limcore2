#include "application.hpp"
#include "device.hpp"

#include <iostream>

int main()
{
	//Limcore::Result<VkInstance> result = Limcore::CreateInstance();
	//if (!result)
	//{
	//	std::cerr << result.error().message << std::endl;
	//	return (0);
	//}
	//VkInstance instance = std::move(*result);

	VkInstance instance = Limcore::CreateInstance().value();

	std::vector<Limcore::DeviceInfo> availableDevices = Limcore::GetAvailableDevices(instance);
	for (const Limcore::DeviceInfo& deviceInfo : availableDevices) {std::cout << deviceInfo << std::endl;}

	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(availableDevices[1].physicalDevice, &queueCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(availableDevices[1].physicalDevice, &queueCount, queueFamilies.data());
	for (const VkQueueFamilyProperties& queueFamilyProperties : queueFamilies) {std::cout << queueFamilyProperties << std::endl;}

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();

	std::cout << "instance destroyed." << std::endl;

	return (0);
}
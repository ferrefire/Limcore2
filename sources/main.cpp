#include "application.hpp"
#include "device.hpp"
#include "window.hpp"

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
	//for (const Limcore::DeviceInfo& deviceInfo : availableDevices) {std::cout << deviceInfo << std::endl;}

	//uint32_t queueCount;
	//vkGetPhysicalDeviceQueueFamilyProperties(availableDevices[1].physicalDevice, &queueCount, nullptr);
	//std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
	//vkGetPhysicalDeviceQueueFamilyProperties(availableDevices[1].physicalDevice, &queueCount, queueFamilies.data());
	//for (const VkQueueFamilyProperties& queueFamilyProperties : queueFamilies) {std::cout << queueFamilyProperties << std::endl;}

	Limcore::WindowConfig windowConfig{};
	Limcore::Window window;
	auto windowCreation = window.Create(instance, availableDevices[1].physicalDevice, windowConfig);
	if (!windowCreation) {windowCreation.error().Print();}

	Limcore::Device device;
	auto deviceCreation = device.Create(instance, availableDevices[1].physicalDevice, window.GetSurface(), {});
	if (!deviceCreation) {deviceCreation.error().Print();}
	
	while (true)
	{
		glfwPollEvents();
		if (window.ShouldClose()) {break;}
	}

	device.Destroy();

	window.Destroy();

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();

	std::cout << "End" << std::endl;

	return (0);
}
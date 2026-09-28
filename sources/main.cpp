#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"

#include <iostream>

int main()
{
	VkInstance instance = Limcore::CreateInstance().value();

	std::vector<Limcore::DeviceInfo> availableDevices = Limcore::GetAvailableDevices(instance);

	Limcore::WindowConfig windowConfig{};
	windowConfig.log = true;
	windowConfig.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	Limcore::Window window;
	auto windowCreation = window.Create(instance, availableDevices[1].physicalDevice, windowConfig);
	if (!windowCreation) {windowCreation.error().Print();}

	Limcore::DeviceFeatures deviceFeatures{};
	deviceFeatures.synchronization2 = true;
	Limcore::Device device;
	auto deviceCreation = device.Create(instance, availableDevices[1].physicalDevice, window.GetSurface(), deviceFeatures);
	if (!deviceCreation) {deviceCreation.error().Print();}

	Limcore::Viewport viewport;
	auto viewportCreation = viewport.Create(device, window, true);
	if (!viewportCreation) {viewportCreation.error().Print();}
	
	while (true)
	{
		glfwPollEvents();
		if (window.ShouldClose()) {break;}
	}

	viewport.Destroy();

	window.Destroy();

	device.Destroy();

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();

	std::cout << "End" << std::endl;

	return (0);
}
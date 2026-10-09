#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"
#include "command.hpp"
#include "error.hpp"
#include "point.hpp"

#include <iostream>
#include <vector>

VkInstance instance;
LC::Device device;
std::vector<LC::Application> applications;

void Clean()
{
	for (LC::Application& application : applications) {application.Destroy();}
	applications.clear();

	device.Destroy();

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();
}

int main()
{
	LC::point3D p(0, 1, 0);
	std::cout << p << std::endl;
	std::cout << p.Length() << std::endl;
	p = {1, 1};
	std::cout << p << std::endl;
	std::cout << p.Length() << std::endl;
	p.Normalize();
	std::cout << p << std::endl;
	std::cout << p.Length() << std::endl;

	p.x() += 2;
	std::cout << p << std::endl;
	p += {-1, 1, 1};
	std::cout << p << std::endl;

	LC::point3D p2(3, 3, 3);
	p += p2;
	std::cout << p << std::endl;

	return (0);

	instance = LC::CreateInstance().value();

	LC::DeviceFeatures deviceFeatures{};
	deviceFeatures.synchronization2 = true;
	deviceFeatures.dynamicRendering = true;

	auto deviceSelection = GetDevice(instance, LC::DeviceType::Best, deviceFeatures);
	if (!deviceSelection) {deviceSelection.error().Print();}
	LC::DeviceInfo selectedDevice = deviceSelection.value();
	auto deviceCreation = device.Create(instance, selectedDevice.physicalDevice, deviceFeatures);
	if (!deviceCreation) {deviceCreation.error().Print();}

	LC::WindowConfig windowConfig{};
	windowConfig.log = true;
	windowConfig.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

	LC::RendererConfig rendererConfig{};
	rendererConfig.log = true;

	for (size_t i = 0; i < 1; i++) {applications.push_back(device);}

	for (LC::Application& application : applications)
	{
		LC::Result applicationCreation = application.Create(instance, windowConfig, rendererConfig);
		if (!applicationCreation) {applicationCreation.error().Print();}
	}
	
	while (true)
	{
		glfwPollEvents();

		bool allClosed = true;
		LC::Result result;

		for (LC::Application& application : applications)
		{
			if (application.IsActive())
			{
				allClosed = false;
				result = application.Frame();
				if (!result) {result.error().Print(); break;}
			}
		}

		if (!result || allClosed) {break;}
	}

	Clean();

	std::cout << "End" << std::endl;

	return (0);
}
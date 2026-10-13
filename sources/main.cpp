#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"
#include "command.hpp"
#include "error.hpp"
#include "point.hpp"
#include "mesh.hpp"

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
	LC::point3D p(1, 2, 3);
	std::cout << p << ": " << sizeof(p) << std::endl;
	LC::point2D p2(1, 2);
	std::cout << p2 << ": " << sizeof(p2) << std::endl;
	LC::dpoint3D p3(1, 2, 3);
	std::cout << p3 << ": " << sizeof(p3) << std::endl;

	LC::Vertex<LC::Position | LC::Normal> v;
	v.position = p;
	std::cout << sizeof(v) << std::endl;

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
#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"
#include "command.hpp"
#include "error.hpp"

#include <iostream>
#include <vector>

VkInstance instance;
Limcore::Device device;
std::vector<Limcore::Application> applications;

void Clean()
{
	for (Limcore::Application& application : applications) {application.Destroy();}
	applications.clear();

	device.Destroy();

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();
}

int main()
{
	instance = Limcore::CreateInstance().value();

	Limcore::DeviceFeatures deviceFeatures{};
	deviceFeatures.synchronization2 = true;
	deviceFeatures.dynamicRendering = true;

	auto deviceSelection = GetDevice(instance, Limcore::DeviceType::Best, deviceFeatures);
	if (!deviceSelection) {deviceSelection.error().Print();}
	Limcore::DeviceInfo selectedDevice = deviceSelection.value();
	auto deviceCreation = device.Create(instance, selectedDevice.physicalDevice, deviceFeatures);
	if (!deviceCreation) {deviceCreation.error().Print();}

	Limcore::WindowConfig windowConfig{};
	windowConfig.log = true;
	windowConfig.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

	Limcore::RendererConfig rendererConfig{};
	rendererConfig.log = true;

	for (size_t i = 0; i < 1; i++) {applications.push_back(device);}

	for (Limcore::Application& application : applications)
	{
		Limcore::Result applicationCreation = application.Create(instance, windowConfig, rendererConfig);
		if (!applicationCreation) {applicationCreation.error().Print();}
	}
	
	while (true)
	{
		glfwPollEvents();

		bool allClosed = true;
		Limcore::Result result;

		for (Limcore::Application& application : applications)
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
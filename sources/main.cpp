#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"

#include <iostream>

int main()
{
	VkInstance instance = Limcore::CreateInstance().value();

	//std::vector<Limcore::DeviceInfo> availableDevices = Limcore::GetAvailableDevices(instance);
	//for (const Limcore::DeviceInfo& deviceInfo : availableDevices) {std::cout << deviceInfo << std::endl;}

	Limcore::DeviceFeatures deviceFeatures{};
	deviceFeatures.synchronization2 = true;
	deviceFeatures.dynamicRendering = true;
	auto deviceSelection = Limcore::GetDevice(instance, Limcore::DeviceType::Best, deviceFeatures);
	if (!deviceSelection) {deviceSelection.error().Print();}
	Limcore::DeviceInfo selectedDevice = deviceSelection.value();
	std::cout << selectedDevice;

	Limcore::WindowConfig windowConfig{};
	windowConfig.log = true;
	windowConfig.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	Limcore::Window window;
	auto windowCreation = window.Create(instance, selectedDevice.physicalDevice, windowConfig);
	if (!windowCreation) {windowCreation.error().Print();}
	std::cout << window;

	Limcore::Device device;
	auto deviceCreation = device.Create(instance, selectedDevice.physicalDevice, window.GetSurface(), deviceFeatures);
	if (!deviceCreation) {deviceCreation.error().Print();}

	Limcore::Viewport viewport;
	auto viewportCreation = viewport.Create(device, window, true);
	if (!viewportCreation) {viewportCreation.error().Print();}
	std::cout << "Swapchain size: " << viewport.GetImages().size() << std::endl;

	Limcore::RendererConfig rendererConfig{};
	rendererConfig.log = true;
	Limcore::Renderer renderer;
	auto rendererCreation = renderer.Create(device, rendererConfig);
	if (!rendererCreation) {rendererCreation.error().Print();}

	for (size_t i = 0; i < viewport.GetImages().size(); i++)
	{
		glfwPollEvents();
		auto frameWait = renderer.WaitForFrame();
		if (!frameWait) {frameWait.error().Print();}
		auto frameRecord = renderer.RecordCommands(viewport, device, true);
		if (!frameRecord) {frameRecord.error().Print();}
	}
	
	while (true)
	{
		glfwPollEvents();
		if (window.ShouldClose()) {break;}
		auto frameWait = renderer.WaitForFrame();
		if (!frameWait) {frameWait.error().Print();}
		auto frameRecord = renderer.RecordCommands(viewport, device);
		if (!frameRecord) {frameRecord.error().Print();}
	}

	renderer.Destroy();

	viewport.Destroy();

	window.Destroy();

	device.Destroy();

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();

	std::cout << "End" << std::endl;

	return (0);
}
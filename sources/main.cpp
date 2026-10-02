#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"
#include "command.hpp"

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
	
	while (true)
	{
		glfwPollEvents();
		if (window.ShouldClose()) {break;}

		Limcore::Result result = renderer.WaitForFrame();
		if (!result) {result.error().Print(); break;}

		result = renderer.BeginFrame(viewport);
		if (!result) {result.error().Print(); break;}

		viewport.TransitionImageToColor(renderer.GetPresentIndex(), renderer.GetCommandBuffer());

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = viewport.GetView(renderer.GetPresentIndex());
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {{1.0f, 1.0f, 1.0f, 1.0f}};

		VkRenderingInfo renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = {{0, 0}, viewport.GetExtent()};
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		vkCmdBeginRendering(renderer.GetCommandBuffer(), &renderInfo);
		vkCmdEndRendering(renderer.GetCommandBuffer());

		viewport.TransitionImageToPresent(renderer.GetPresentIndex(), renderer.GetCommandBuffer());

		result = renderer.EndFrame(device, viewport);
		if (!result) {result.error().Print(); break;}

		result = renderer.PresentFrame(device, viewport);
		if (!result) {result.error().Print(); break;}
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
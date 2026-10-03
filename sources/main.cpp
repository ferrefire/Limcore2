#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"
#include "command.hpp"
#include "error.hpp"

#include <iostream>

bool Frame(Limcore::Window& window, Limcore::Device& device, Limcore::Viewport& viewport, Limcore::Renderer& renderer, VkClearColorValue clearValue)
{
	Limcore::Result result = renderer.WaitForFrame();
	if (!result) {result.error().Print(); return (false);}

	if (renderer.SwapchainOutOfDate())
	{
		window.Resized();
		result = viewport.Recreate(device, window);
		if (!result) {result.error().Print(); return (false);}
	}

	result = renderer.BeginFrame(viewport);
		
	while (!result && renderer.SwapchainOutOfDate())
	{
		window.Resized();
		result = viewport.Recreate(device, window);
		if (!result) {result.error().Print(); return (false);}
		result = renderer.BeginFrame(viewport);
	}
	if (!result) {result.error().Print(); return (false);}

	viewport.TransitionImageToColor(renderer.GetPresentIndex(), renderer.GetCommandBuffer());

	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = viewport.GetView(renderer.GetPresentIndex());
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue = {clearValue};

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
	if (!result) {result.error().Print(); return (false);}

	result = renderer.PresentFrame(device, viewport);
	if (!result) {result.error().Print(); return (false);}

	return (true);
}

int main()
{
	VkInstance instance = Limcore::CreateInstance().value();
	Limcore::Window window;
	Limcore::Device device;
	Limcore::Viewport viewport;
	Limcore::Renderer renderer;

	Limcore::Window window2;
	Limcore::Viewport viewport2;
	Limcore::Renderer renderer2;

	Limcore::Window window3;
	Limcore::Viewport viewport3;
	Limcore::Renderer renderer3;

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
	auto windowCreation = window.Create(instance, selectedDevice.physicalDevice, windowConfig);
	if (!windowCreation) {windowCreation.error().Print();}
	windowCreation = window2.Create(instance, selectedDevice.physicalDevice, windowConfig);
	if (!windowCreation) {windowCreation.error().Print();}
	windowCreation = window3.Create(instance, selectedDevice.physicalDevice, windowConfig);
	if (!windowCreation) {windowCreation.error().Print();}
	std::cout << window;

	auto deviceCreation = device.Create(instance, selectedDevice.physicalDevice, window.GetSurface(), deviceFeatures);
	if (!deviceCreation) {deviceCreation.error().Print();}

	auto viewportCreation = viewport.Create(device, window, true);
	if (!viewportCreation) {viewportCreation.error().Print();}
	viewportCreation = viewport2.Create(device, window2, true);
	if (!viewportCreation) {viewportCreation.error().Print();}
	viewportCreation = viewport3.Create(device, window3, true);
	if (!viewportCreation) {viewportCreation.error().Print();}
	//std::cout << "Swapchain size: " << viewport.GetImages().size() << std::endl;

	Limcore::RendererConfig rendererConfig{};
	rendererConfig.log = true;
	auto rendererCreation = renderer.Create(device, rendererConfig);
	if (!rendererCreation) {rendererCreation.error().Print();}
	rendererCreation = renderer2.Create(device, rendererConfig);
	if (!rendererCreation) {rendererCreation.error().Print();}
	rendererCreation = renderer3.Create(device, rendererConfig);
	if (!rendererCreation) {rendererCreation.error().Print();}
	
	while (true)
	{
		glfwPollEvents();
		if (window.ShouldClose()) {break;}
		if (!Frame(window, device, viewport, renderer, {1.0f, 0.0f, 0.0f, 1.0f})) {break;}
		if (window2.ShouldClose()) {break;}
		if (!Frame(window2, device, viewport2, renderer2, {0.0f, 1.0f, 0.0f, 1.0f})) {break;}
		if (window3.ShouldClose()) {break;}
		if (!Frame(window3, device, viewport3, renderer3, {0.0f, 0.0f, 1.0f, 1.0f})) {break;}
	}

	renderer.Destroy();
	renderer2.Destroy();
	renderer3.Destroy();

	viewport.Destroy();
	viewport2.Destroy();
	viewport3.Destroy();

	window.Destroy();
	window2.Destroy();
	window3.Destroy();

	device.Destroy();

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();

	std::cout << "End" << std::endl;

	return (0);
}
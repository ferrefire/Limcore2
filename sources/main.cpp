#include "application.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"
#include "command.hpp"
#include "error.hpp"

#include <iostream>

VkInstance instance;
Limcore::Window window;
Limcore::Device device;
Limcore::Viewport viewport;
Limcore::Renderer renderer;

bool Frame(Limcore::Window& window, Limcore::Device& device, Limcore::Viewport& viewport, Limcore::Renderer& renderer)
{
	Limcore::Result result = renderer.WaitForFrame();
	if (!result) {result.error().Print(); return (false);}

	result = renderer.BeginFrame(viewport);
	if (!result) {result.error().Print(); return (false);}

	result = renderer.RenderFrame();
	if (!result) {result.error().Print(); return (false);}

	result = renderer.EndFrame(device, viewport);
	if (!result) {result.error().Print(); return (false);}

	result = renderer.PresentFrame(device, viewport);
	if (!result) {result.error().Print(); return (false);}

	return (true);
}

void RecreateSwapchain()
{
	window.Resized();
	auto result = viewport.Recreate(device, window);
	if (!result) {result.error().Print();}
}

void Clean()
{
	renderer.Destroy();
	viewport.Destroy();
	window.Destroy();
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
	auto deviceSelection = Limcore::GetDevice(instance, Limcore::DeviceType::Best, deviceFeatures);
	if (!deviceSelection) {deviceSelection.error().Print();}
	Limcore::DeviceInfo selectedDevice = deviceSelection.value();
	std::cout << selectedDevice;

	Limcore::WindowConfig windowConfig{};
	windowConfig.log = true;
	windowConfig.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	auto windowCreation = window.Create(instance, selectedDevice.physicalDevice, windowConfig);
	if (!windowCreation) {windowCreation.error().Print();}
	std::cout << window;

	auto deviceCreation = device.Create(instance, selectedDevice.physicalDevice, window.GetSurface(), deviceFeatures);
	if (!deviceCreation) {deviceCreation.error().Print();}

	auto viewportCreation = viewport.Create(device, window, true);
	if (!viewportCreation) {viewportCreation.error().Print();}

	Limcore::RendererConfig rendererConfig{};
	rendererConfig.swapchainRecreateCallback = RecreateSwapchain;
	rendererConfig.log = true;
	auto rendererCreation = renderer.Create(device, rendererConfig);
	if (!rendererCreation) {rendererCreation.error().Print();}

	Limcore::RendererAttachment colorAttachment{};
	colorAttachment.views = &viewport.GetViews();
	colorAttachment.indexType = Limcore::AttachmentIndexType::PresentIndex;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	Limcore::RendererPass rendererPass{};
	rendererPass.colorAttachments.push_back(colorAttachment);
	rendererPass.RegisterPreRenderingCall(&viewport, &Limcore::Viewport::TransitionImageToColor);
	rendererPass.RegisterPostRenderingCall(&viewport, &Limcore::Viewport::TransitionImageToPresent);
	rendererPass.extent = &viewport.GetExtent();

	renderer.AddRendererPass(rendererPass);
	
	while (true)
	{
		glfwPollEvents();
		if (window.ShouldClose()) {break;}
		if (!Frame(window, device, viewport, renderer)) {break;}
	}

	Clean();

	std::cout << "End" << std::endl;

	return (0);
}
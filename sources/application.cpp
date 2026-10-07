#include "application.hpp"

#include "printer.hpp"
#include "utility.hpp"

#include <iostream>
#include <cassert>
#include <string>

namespace Limcore
{
	Result Application::Create(const VkInstance& instance, DeviceType deviceType, DeviceFeatures deviceFeatures, WindowConfig windowConfig, RendererConfig rendererConfig)
	{
		const std::string message = "Failed to create application";

		auto deviceSelection = GetDevice(instance, deviceType, deviceFeatures);
		RETURN_ERROR(deviceSelection, message)
		DeviceInfo selectedDevice = deviceSelection.value();

		Result result = window.Create(instance, selectedDevice.physicalDevice, windowConfig);
		RETURN_ERROR(result, message)

		result = device.Create(instance, selectedDevice.physicalDevice, window.GetSurface(), deviceFeatures);
		RETURN_ERROR(result, message)

		result = viewport.Create(device, window);
		RETURN_ERROR(result, message)

		result = renderer.Create(device, BindFunction(this, &Application::RecreateViewport), rendererConfig);
		RETURN_ERROR(result, message)

		RendererAttachment colorAttachment{};
		colorAttachment.views = &viewport.GetViews();
		colorAttachment.indexType = AttachmentIndexType::PresentIndex;
		colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		RendererPass rendererPass{};
		rendererPass.colorAttachments.push_back(colorAttachment);
		rendererPass.RegisterPreRenderingCall(&viewport, &Viewport::TransitionImageToColor);
		rendererPass.RegisterPostRenderingCall(&viewport, &Viewport::TransitionImageToPresent);
		rendererPass.extent = &viewport.GetExtent();
		renderer.AddRendererPass(rendererPass);

		active = true;

		return (Result());
	}

	void Application::Destroy() noexcept
	{
		active = false;

		renderer.Destroy();
		viewport.Destroy();
		window.Destroy();
		device.Destroy();
	}

	void Application::RecreateViewport()
	{
		window.Resized();
		Result result = viewport.Recreate(device, window);
		if (!result) {result.error().Print();}
	}

	Result Application::Frame()
	{
		const std::string message = "Failed to execute application frame";

		if (window.ShouldClose()) {Destroy(); return (Result());}

		Result result = renderer.WaitForFrame();
		RETURN_ERROR(result, message)

		result = renderer.BeginFrame(viewport);
		RETURN_ERROR(result, message)

		result = renderer.RenderFrame();
		RETURN_ERROR(result, message)

		result = renderer.EndFrame(device, viewport);
		RETURN_ERROR(result, message)

		result = renderer.PresentFrame(device, viewport);
		RETURN_ERROR(result, message)

		return (Result());
	}

	bool HasValidationLayers(const std::vector<const char*>& layers)
	{
		uint32_t layerCount = 0;
		if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS || layerCount == 0) {return (false);}
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layer : layers)
		{
			bool layerFound = false;
			for (const VkLayerProperties& availableLayer : availableLayers)
			{
				if (std::string(availableLayer.layerName).compare(layer) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {return (false);}
		}

		return (true);
	}

	ResultT<VkInstance> CreateInstance()
	{
		if (!glfwInit())
		{
			Error error{ErrorCode::GlfwError, "glfw initialization failed"};
			return (std::unexpected(error));
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		std::cout << "glfw initialized" << std::endl;

		uint32_t instanceVersion = VK_API_VERSION_1_0;
		PFN_vkEnumerateInstanceVersion pfnEnumerateInstanceVersion =
			reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
		if (pfnEnumerateInstanceVersion) {pfnEnumerateInstanceVersion(&instanceVersion);}
		std::cout << "Vulkan instance version: " << instanceVersion << " "
			<< VK_API_VERSION_MAJOR(instanceVersion) << "."
			<< VK_API_VERSION_MINOR(instanceVersion) << "."
			<< VK_API_VERSION_PATCH(instanceVersion) << std::endl;

		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = "Limcore";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pEngineName = "None";
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0);

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		if (glfwExtensions == nullptr) {return (std::unexpected(Error(ErrorCode::GlfwError, "glfw required extensions not found")));}

		std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
		bool validationLayersFound = HasValidationLayers(layers);

		std::cout << "Vulkan validation layers found: " << validationLayersFound << std::endl;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = validationLayersFound ? CUI(layers.size()) : 0;
		createInfo.ppEnabledLayerNames = createInfo.enabledLayerCount > 0 ? layers.data() : nullptr;
		createInfo.pApplicationInfo = &applicationInfo;

		VkInstance instance = nullptr;
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result != VK_SUCCESS || instance == nullptr) {return (std::unexpected(Error(ErrorCode::VulkanError, "Vulkan instance creation failed", result)));}

		std::cout << "Vulkan instance created" << std::endl;

		return (instance);
	}
}
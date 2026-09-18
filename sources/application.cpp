#include "application.hpp"

#include <iostream>
#include <cassert>

namespace Limcore
{
	Application::Application(const VkInstance& instance) noexcept : instance(instance)
	{
		assert(instance != nullptr);
	}

	void Application::Destroy() noexcept
	{

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

	Result<VkInstance> CreateInstance()
	{
		if (!glfwInit())
		{
			Error error{ErrorCode::GlfwError, "glfw initialization failed."};
			return (std::unexpected(error));
		}

		std::cout << "glfw initialized." << std::endl;

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

		if (glfwExtensions == nullptr)
		{
			Error error{ErrorCode::GlfwError, "glfw required extensions not found."};
			return (std::unexpected(error));
		}

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
		if (result != VK_SUCCESS || instance == nullptr)
		{
			Error error{ErrorCode::VulkanError, std::string("Vulkan instance creation failed with result: ") + std::to_string(result)};
			return (std::unexpected(error));
		}

		std::cout << "Vulkan instance created." << std::endl;

		return (instance);
	}
}
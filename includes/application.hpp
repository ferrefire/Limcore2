#pragma once

#include "error.hpp"
#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"
#include "renderer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <utility>

namespace Limcore
{
	class Application
	{
		private:
			bool active = false;

			Device device;
			Window window;
			Viewport viewport;
			Renderer renderer;

		public:
			Application() noexcept = default;
			~Application() noexcept {Destroy();}

			Application(const Application&) = delete;
			Application& operator=(const Application&) = delete;

			Application(Application&& other) noexcept :
				active(std::exchange(other.active, false)),
				device(std::exchange(other.device, {})),
				window(std::exchange(other.window, {})),
				viewport(std::exchange(other.viewport, {})),
				renderer(std::exchange(other.renderer, {})) {}
			
			Application& operator=(Application&& other) noexcept
			{
				if (this != &other)
				{
					Destroy();
					active = std::exchange(other.active, false);
					device = std::exchange(other.device, {});
					window = std::exchange(other.window, {});
					viewport = std::exchange(other.viewport, {});
					renderer = std::exchange(other.renderer, {});
				}
				return (*this);
			}

			[[nodiscard]] Result Create(const VkInstance& instance, DeviceType deviceType, DeviceFeatures deviceFeatures, WindowConfig windowConfig, RendererConfig rendererConfig);

			void Destroy() noexcept;

			void RecreateViewport();

			Result Frame();

			[[nodiscard]] const bool& IsActive() const noexcept {return (active);}
	};

	[[nodiscard]] bool HasValidationLayers(const std::vector<const char*>& layers);
	[[nodiscard]] ResultT<VkInstance> CreateInstance(); //Todo: Add a config struct as parameter.
}
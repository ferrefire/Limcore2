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
			const Device& device;
			Window window;
			Viewport viewport;
			Renderer renderer;

			bool active = false;

		public:
			Application(const Device& device) noexcept : device(device) {}
			~Application() noexcept {Destroy();}

			Application(const Application&) = delete;
			Application& operator=(const Application&) = delete;

			Application(Application&& other) noexcept :
				device(other.device),
				window(std::exchange(other.window, {})),
				viewport(std::exchange(other.viewport, {})),
				renderer(std::exchange(other.renderer, {})),
				active(std::exchange(other.active, false)) {}
			
			Application& operator=(Application&& other) = delete;

			[[nodiscard]] Result Create(const VkInstance& instance, WindowConfig windowConfig, RendererConfig rendererConfig);

			void Destroy() noexcept;

			void RecreateViewport();

			Result Frame();

			[[nodiscard]] const bool& IsActive() const noexcept {return (active);}
	};

	/**
	 * @brief Checks if the given validation layers are present on the machine.
	 * @param layers The layers to check.
	 * @return True if the layers are present.
	 */
	[[nodiscard]] bool HasValidationLayers(const std::vector<const char*>& layers);

	/**
	 * @brief Creates a Vulkan instance and initializes GLFW.
	 * @return Result containing the created instance or an error.
	 * @todo Add a config struct as parameter.
	 */
	[[nodiscard]] ResultT<VkInstance> CreateInstance();
}

namespace LC = Limcore;
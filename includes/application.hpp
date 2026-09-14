#pragma once

#include "error.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#define CUI(a) static_cast<uint32_t>(a)

namespace Limcore
{
	class Application
	{
		private:
			const VkInstance& instance;

		public:
			Application(const VkInstance& instance) noexcept;
			~Application() noexcept {Destroy();}

			Application(const Application&) = delete;
			Application& operator=(const Application&) = delete;

			//Implement move operators.

			void Destroy() noexcept;
	};

	[[nodiscard]] bool HasValidationLayers(const std::vector<const char*>& layers);
	[[nodiscard]] Result<VkInstance> CreateInstance(); //Add a config struct as parameter.
}
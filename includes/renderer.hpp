#pragma once

#include "device.hpp"
#include "window.hpp"
#include "viewport.hpp"

namespace Limcore
{
	struct RendererConfig
	{
		uint32_t maxFramesInFlight = 2;
		bool log = false;
	};

	class Renderer
	{
		private:
			Device& device;
			Window& window;
			Viewport& viewport;

			RendererConfig config{};
			uint32_t frameInFlightIndex = 0;

		public:
			Renderer(Device& device, Window& window, Viewport& viewport, RendererConfig rendererConfig) noexcept;
			~Renderer() noexcept {Destroy();}

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			//Todo: implement move operators.

			void Destroy() noexcept;
	};
}
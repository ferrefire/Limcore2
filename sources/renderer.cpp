#include "renderer.hpp"

#include <cassert>

namespace Limcore
{
	Renderer::Renderer(Device& device, Window& window, Viewport& viewport, RendererConfig rendererConfig) noexcept : device(device), window(window), viewport(viewport), config(rendererConfig)
	{
		assert(device.IsValid());
		assert(window.IsValid());
		assert(viewport.IsValid());
		assert(rendererConfig.maxFramesInFlight >= 1 && rendererConfig.maxFramesInFlight <= 3);
	}
}
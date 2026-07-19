#include "pch.h"
#include "renderer/rendererAPI.h"

namespace AppWMs {
	RendererAPI::SelectRenderer RendererAPI::s_render = 
		RendererAPI::SelectRenderer::Vulkan;
}
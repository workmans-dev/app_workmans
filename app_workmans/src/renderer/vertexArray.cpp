#include "pch.h"

#include "renderer/vertexArray.h"
#include "renderer/renderer.h"

namespace AppWMs {

	Ref<VertexArray> VertexArray::create()
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::Vulkan: return std::make_shared<OpenGLVertexArray>();
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}
}
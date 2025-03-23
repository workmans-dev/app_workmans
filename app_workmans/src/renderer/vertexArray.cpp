#include "pch.h"

#include "renderer/vertexArray.h"
#include "openGL/openGLVertexArray.h"
#include "renderer/renderer.h"

namespace AppWMs {

	Ref<VertexArray> VertexArray::create()
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return std::make_shared<OpenGLVertexArray>();
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}
}
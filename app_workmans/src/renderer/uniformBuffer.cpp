#include "pch.h"

#include "renderer/uniformBuffer.h"
#include "openGL/openGLuniformBuffer.h"
#include "renderer/renderer.h"


namespace AppWMs {
	Ref<UniformBuffer> UniformBuffer::create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return std::make_shared<OpenGLuniformBuffer>(size, binding);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}
}
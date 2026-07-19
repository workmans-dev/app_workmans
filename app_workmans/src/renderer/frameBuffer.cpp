#include "pch.h"

#include "renderer/frameBuffer.h"
#include "renderer/renderer.h"

namespace AppWMs {
	Ref<FrameBuffer> FrameBuffer::create(const FrameBufferSpec& spec)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return std::make_shared<OpenGLframeBuffer>(spec);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

}
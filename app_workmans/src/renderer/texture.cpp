
#include "pch.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "openGL/openGLTexture2D.h"

namespace AppWMs {
	Ref<Texture2D> create(const std::string& path)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}
}

#include "pch.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "openGL/openGLTexture2D.h"

namespace AppWMs {
	Ref<Texture2D> Texture2D::create(uint32_t w, uint32_t h)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return std::make_shared<OpenGLTexture2D>(w, h);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::create(const std::string& path)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

}
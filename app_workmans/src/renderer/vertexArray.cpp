#include "pch.h"

#include "renderer/vertexArray.h"
#include "openGL/openGLVertexArray.h"
#include "renderer/renderer.h"

namespace AppWMs {
	VertexArray::~VertexArray()
	{
	}
	void VertexArray::bind() const
	{
	}
	void VertexArray::unbind() const
	{
	}
	void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer> vertexBuffer)
	{
	}
	void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer> indexBuffer)
	{
	}
	VertexArray* VertexArray::create()
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return new OpenGLVertexArray();
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}
}
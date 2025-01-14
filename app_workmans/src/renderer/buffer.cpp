#include "pch.h"

#include "buffer.h"

#include "openGL/openGLVertexBuffer.h"
#include "renderer/renderer.h"

//#if APP_WMS_PLATFORM_WINDOWS
//#include "direct3D/direct3DBuffer.h"
//#endif

namespace AppWMs {


	/// <summary>
	/// VertexBuffer
	/// </summary>
	VertexBuffer::~VertexBuffer()
	{
	}

	VertexBuffer* VertexBuffer::create(float* vertices, uint32_t size)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return new OpenGLVertexBuffer(vertices, size);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}


	/// <summary>
	/// IndexBuffer
	/// </summary>
	IndexBuffer::~IndexBuffer()
	{
	}

	IndexBuffer* IndexBuffer::create(uint32_t* indexes, uint32_t count)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: return new OpenGLIndexBuffer(indexes, count);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

	/// <summary>
	/// BufferLayout
	/// </summary>
	BufferLayout::BufferLayout()
	{
	}
}
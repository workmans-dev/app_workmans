#include "pch.h"

#include "buffer.h"
#include "renderer/renderer.h"

//#if APP_WMS_PLATFORM_WINDOWS
//#include "direct3D/direct3DBuffer.h"
//#endif

#include "vulkan/vk_buffer.h"


namespace AppWMs {


	/// <summary>
	/// VertexBuffer
	/// </summary>


	Ref<VertexBuffer> VertexBuffer::create(float* vertices, uint32_t size)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None:
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
			return nullptr;   // ⚠️ ASSERTS는 Release에서 컴파일 아웃 → fall-through 방지
		case RendererAPI::SelectRenderer::Vulkan:
			return std::make_shared<VulkanVertexBuffer>(vertices, size);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::create(uint32_t size)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None:
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
			return nullptr;
		case RendererAPI::SelectRenderer::Vulkan:
			return std::make_shared<VulkanVertexBuffer>( size);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}









	/// <summary>
	/// IndexBuffer
	/// </summary>


	Ref<IndexBuffer> IndexBuffer::create(uint32_t* indexes, uint32_t count)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None:
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
			return nullptr;
		case RendererAPI::SelectRenderer::Vulkan:
			return std::make_shared<VulkanIndexBuffer>(indexes, count);
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
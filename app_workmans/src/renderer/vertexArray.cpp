#include "pch.h"

#include "renderer/vertexArray.h"
#include "renderer/renderer.h"
#include "vulkan/vk_vertexArray.h"
namespace AppWMs {

	Ref<VertexArray> VertexArray::create()
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None:
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
			return nullptr;
		case RendererAPI::SelectRenderer::Vulkan:
			return std::make_shared<VulkanVertexArray>();
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}
}
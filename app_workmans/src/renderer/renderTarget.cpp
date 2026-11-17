#include "pch.h"

#include "renderer/renderTarget.h"
#include "renderer/renderer.h"
#include "vulkan/vk_renderTarget.h"

namespace AppWMs {
	Ref<RenderTarget> RenderTarget::create(const RenderTargetSpec& spec)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None:
			// WARNING: the return is required -- ASSERTS compiles out in Release, so
			//    without it None falls through into the Vulkan case below (a defect inherited from Hazel)
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
			return nullptr;
		case RendererAPI::SelectRenderer::Vulkan:
			return std::make_shared<VulkanRenderTarget>(spec);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

}

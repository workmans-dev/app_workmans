#include "pch.h"

#include "renderer/renderTarget.h"
#include "renderer/renderer.h"
#include "vulkan/vk_renderTarget.h"

namespace AppWMs {
	Ref<RenderTarget> RenderTarget::create(const RenderTargetSpec& spec)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None:
			// ⚠️ return 필수 — ASSERTS는 Release에서 컴파일 아웃되므로
			//    이게 없으면 None이 아래 Vulkan case로 흘러든다 (Hazel 물림 결함)
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
			return nullptr;
		case RendererAPI::SelectRenderer::Vulkan:
			return std::make_shared<VulkanRenderTarget>(spec);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

}

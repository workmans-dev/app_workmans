#include "pch.h"
#include "rendererCommand.h"

#include "vulkan/vulkanRenderer.h"

namespace AppWMs {

	Scope<RendererAPI> RendererCommand::pm_rendererAPI_ptr =
		std::make_unique<VulkanRendererAPI>();

}
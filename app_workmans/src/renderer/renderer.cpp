#include "pch.h"
#include "renderer/renderer.h"

namespace AppWMs {
	glm::mat4 Renderer::pm_cameraViewProj = glm::mat4(1.0f);

	void Renderer::init()
	{
		RendererCommand::init();
	}

	void Renderer::shutdown()
	{
		// The GPU wait and layer destruction were already done by App before this (see App::~App).
		// Here we only release what the backend owns (render pass / framebuffers / descriptor pool).
		RendererCommand::shutdown();
	}

	void Renderer::onWindowResize(uint32_t w, uint32_t h)
	{
		RendererCommand::set_viewport(0, 0, w, h);
	}

	void Renderer::beginScene(const glm::mat4& viewProj)
	{
		pm_cameraViewProj = viewProj;
	}

	void Renderer::submit(const Ref<VertexArray>& vertexArray, const glm::mat4& model,
		int32_t entityID)
	{
		// MVP is composed here and sent as one push constant per object.
		// View and projection were already multiplied in beginScene, so only the model is added.
		RendererCommand::pushObjectMatrices(pm_cameraViewProj * model, model, entityID);
		RendererCommand::drawIndexed(vertexArray);
	}

	void Renderer::end()
	{
	}
}
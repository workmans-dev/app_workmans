#include "pch.h"
#include "renderer/renderer.h"

namespace AppWMs {
	Renderer::SceneData* Renderer::pm_sceneData = new Renderer::SceneData;

	void Renderer::beginScene(Camera& camera)
	{
		pm_sceneData->view_proj_mat = camera.get_pm_view_proj_mat();
	}
	void Renderer::submit(const Ref<VertexArray>& vertexArray, const Ref<Shader> shader,
		const glm::mat4& transform)
	{
		shader->bind();
		shader->uploadUniformMatrix("u_view_proj", pm_sceneData->view_proj_mat);
		shader->uploadUniformMatrix("u_transform", transform);
		vertexArray->bind();
		RendererCommand::drawIndexed(vertexArray);
	}
	void Renderer::end()
	{
	}
}
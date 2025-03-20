#include "pch.h"
#include "renderer/renderer.h"
#include "openGL/openGLShader.h"
#include "renderer/renderer2D.h"

namespace AppWMs {
	Scope<Renderer::SceneData> Renderer::pm_sceneData = std::make_unique<Renderer::SceneData>();

	void Renderer::init()
	{
		RendererCommand::init();
		Renderer2D::init();
	}

	void Renderer::onWindowResize(uint32_t w, uint32_t h)
	{
		RendererCommand::set_viewport(0, 0, w, h);
	}

	void Renderer::beginScene(const CameraController& camera)
	{
		pm_sceneData->view_proj_mat = camera.get_projection();
	}

	void Renderer::submit(const Ref<VertexArray>& vertexArray, const Ref<Shader> shader,
		const glm::mat4& transform)
	{
		shader->bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->uploadUniformMat4("u_view_proj", pm_sceneData->view_proj_mat);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->uploadUniformMat4("u_transform", transform);
		vertexArray->bind();
		RendererCommand::drawIndexed(vertexArray);
	}
	void Renderer::end()
		
	{
	}
}
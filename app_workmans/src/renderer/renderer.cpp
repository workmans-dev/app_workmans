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
		// GPU 대기와 레이어 파괴는 App이 이 앞에서 이미 끝냈다 (App::~App 참고).
		// 여기선 백엔드가 소유한 리소스(렌더패스/프레임버퍼/디스크립터풀)만 반납한다.
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

	void Renderer::submit(const Ref<VertexArray>& vertexArray, const glm::mat4& model)
	{
		// MVP를 여기서 합성해 오브젝트당 push constant 한 번으로 보낸다.
		// 뷰·투영은 beginScene에서 이미 곱해져 있으므로 모델만 더하면 된다.
		RendererCommand::pushObjectMatrices(pm_cameraViewProj * model, model);
		RendererCommand::drawIndexed(vertexArray);
	}

	void Renderer::end()
	{
	}
}
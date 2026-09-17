#pragma once
#include "renderer/rendererCommand.h"
#include "renderer/camera/cameraController.h"
//#include "renderer/shader.h"

namespace AppWMs {

	class Renderer {
	public:
		static void init();
		static void shutdown();
		static bool beginFrame() { return RendererCommand::beginFrame(); }
		static void endFrame() { RendererCommand::endFrame(); }
		static void onWindowResize(uint32_t w, uint32_t h);
		// start render
		// viewProj는 호출자가 이미 벌칸 보정(proj[1][1] *= -1)을 끝낸 값이어야 한다.
		static void beginScene(const glm::mat4& viewProj);
		// ④ C안 — 파이프라인은 호출자가 미리 바인딩해둔 것을 쓴다.
		// 파이프라인별 정렬 루프가 생기면 그 루프가 바인딩을 관리하게 된다.
		// entityID는 피킹 id 버퍼에 찍힐 값. 기본값 -1은 "없음"(= entt::null의 비트패턴)이라
		// 이 값으로 그린 픽셀은 클릭해도 아무것도 안 잡힌다.
		static void submit(const Ref<VertexArray>& vertexArray,
			const glm::mat4& model = glm::mat4(1.0f),
			int32_t entityID = -1);
		static void end();

		inline static RendererAPI::SelectRenderer getRenderer() { return RendererAPI::getRenderer(); }

	private:
		// 이번 프레임 카메라가 세상을 보는 방식 = Projection × View.
		//
		// ⚠️ Model(=월드 행렬)은 여기 없다. 오브젝트마다 다르므로 submit이 인자로 받는다.
		//    MVP = pm_cameraViewProj * model 로 submit에서 완성된다.
		//
		// 카메라는 프레임당 1개인데 오브젝트는 여럿이라, 여기서 한 번만 합성해두고
		// 오브젝트마다 model만 곱한다. beginScene이 채우고 submit이 읽는다.
		//
		// 조명·카메라 위치처럼 프레임당 공유값이 늘면 그때 struct로 묶는다.
		// 지금은 필드가 하나뿐이라 힙 할당(Scope)을 걷어냈다.
		static glm::mat4 pm_cameraViewProj;
	};
}
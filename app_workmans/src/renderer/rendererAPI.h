#pragma once

#include <glm/glm.hpp>
#include "renderer/vertexArray.h"

namespace AppWMs {
	class RendererAPI {
	public:
		enum class SelectRenderer {
			None = 0, Vulkan = 1
		};
	public:
		virtual void init() = 0;
		// 🟢 프레임 생명주기 (백엔드가 acquire/submit/present를 은닉)
		virtual bool beginFrame() = 0;
		virtual void endFrame() = 0;
		virtual void set_viewport(uint32_t x, uint32_t y,
			uint32_t w, uint32_t h) = 0;
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual void clear() =0;
		virtual void drawIndexed(const Ref<VertexArray>& vertexArray, 
			uint32_t indexCount=0) = 0;
		virtual void drawLines(const Ref<VertexArray>& vertexArray, 
			uint32_t vertexCount) = 0;
		virtual void setLineWidth(float width) = 0;
		// 🟢 오브젝트별 변환 행렬 전달.
		//    벌칸 구현은 현재 바인딩된 파이프라인의 push constant로 보낸다.
		//    머티리얼(UBO/디스크립터셋)이 생기면 이 자리가 그쪽으로 확장된다.
		//    entityID는 피킹용 id 버퍼에 찍힐 값이다. -1이 "없음"이고, 그 비트패턴이
		//    entt::null과 같아서 읽은 값을 그대로 엔티티로 되돌릴 수 있다.
		//    ⚠️ 여기서 entt 타입을 쓰지 않는 건 의도적이다 — 렌더러는 씬을 모른다.
		virtual void pushObjectMatrices(const glm::mat4& mvp, const glm::mat4& model,
			int32_t entityID) = 0;
		// 🟢 GPU가 제출된 작업을 다 끝낼 때까지 대기.
		//    종료 시 리소스 파괴 전에 한 번 불러 사용 중 핸들 파괴를 막는다.
		virtual void waitIdle() = 0;
		// 🟢 백엔드가 소유한 리소스 반납. 소멸자에 맡기지 않는 이유는
		//    구현체가 정적 수명(RendererCommand의 unique_ptr)이라 파괴 시점이
		//    그래픽 디바이스보다 늦기 때문이다. 종료 순서는 App이 잡는다.
		virtual void shutdown() = 0;
		inline static RendererAPI::SelectRenderer getRenderer() { return s_render; }
	private:
		static SelectRenderer s_render;
	};

}
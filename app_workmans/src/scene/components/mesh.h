#pragma once
#include "core/core.h"              // Ref
#include "renderer/vertexArray.h"

namespace AppWMs {

    // "어떤 형상인가" — 폴리곤 메쉬.
    //
    // 엔티티가 자기 메쉬를 소유한다. 씬이 공유본 하나를 돌려쓰면 두 오브젝트를
    // 서로 다르게 편집할 수 없다 — 모델링 앱에선 성립하지 않는 전제다.
    //
    // ⚠️ 프리미티브(cube/plane/sphere)는 컴포넌트 종류가 아니다.
    //    "큐브"는 초기 형상을 찍어내는 팩토리(renderer/geometry/cube.h)일 뿐이고,
    //    만들어진 뒤로는 그냥 편집 가능한 폴리곤 메쉬다. 블렌더와 같은 취급.
    //    (마야는 여기에 construction history를 얹어 파라미터를 다시 만지게 하지만,
    //     그건 파라메트릭 히스토리라는 별개 축이지 형상 종류가 아니다)
    //
    // 지금은 GPU측 VertexArray를 직접 들지만, CPU측 편집 가능한 Mesh 타입이
    // 생기면 Ref<Mesh>로 바뀐다. 컴포넌트의 모양은 그대로다.
    struct MeshComponent {
        static constexpr const char* TypeName() { return "MeshComponent"; }

        Ref<VertexArray> mesh;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
        MeshComponent(const Ref<VertexArray>& mesh) : mesh(mesh) {}
    };

}

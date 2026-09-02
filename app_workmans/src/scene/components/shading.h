#pragma once
#include <string>
#include <glm/glm.hpp>

namespace AppWMs {

    // "어떻게 보이나" — 색·머티리얼과 표시 여부.
    //
    // VisComponent가 identity가 아니라 여기 있는 이유: 이름·메타는 오브젝트가
    // 무엇인지에 대한 정보고, 표시 여부는 그리느냐 마느냐의 렌더 판단이다.
    // (잠금·선택가능처럼 아웃라이너 전용 속성이 늘면 그때 따로 뺀다)

    struct VisComponent {
        static constexpr const char* TypeName() { return "VisComponent"; }

        int visible = true;

        VisComponent(const int& v) :visible(v) {};
        VisComponent(const VisComponent& other) :visible(other.visible) {};
        VisComponent() = default;
    };

    struct ShadingComponent {
        static constexpr const char* TypeName() { return "ShadingComponent"; }

        glm::vec4 color{ 0.0f, 1.0f, 1.0f , 1.0f };

        ShadingComponent() = default;
        ShadingComponent(const ShadingComponent&) = default;
        ShadingComponent(const glm::vec4& color) :color(color) {};
    };

}

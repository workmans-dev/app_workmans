#pragma once
#include <string>
#include <glm/glm.hpp>

namespace AppWMs {

    // "How does it look" -- color, material and visibility.
    //
    // Why VisComponent lives here rather than in identity: name and metadata say what the
    // object is, while visibility is the render decision of whether to draw it.
    // (Once outliner-only attributes such as locked/selectable pile up, they get split out then.)

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

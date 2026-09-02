#pragma once
#include <string>

namespace AppWMs {

    // "이건 무엇인가" — 이름과 메타데이터.
    // 표시 여부(VisComponent)는 렌더 속성이라 shading.h로 갔다.
    struct TagComponent {
        // 직렬화 키 = 컴파일 타임 상수. 인스턴스 크기 0바이트.
        static constexpr const char* TypeName() { return "TagComponent"; }

        std::string name;
        bool nameEditable = true;

        TagComponent() = default;
        TagComponent(const std::string& name) :name(name) {};
        TagComponent(const TagComponent&) = default;
    };

}

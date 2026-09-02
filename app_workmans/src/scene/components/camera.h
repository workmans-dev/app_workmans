#pragma once
#include <string>
#include "renderer/camera/sceneCamera.h"

namespace AppWMs {

    // "어떻게 보는가" — 이 엔티티를 카메라로 만드는 컴포넌트.
    //
    // ⚠️ 뷰 행렬은 여기 없다. 카메라도 다른 오브젝트와 같은 TransformComponent를
    //    쓰고, 뷰는 그 역행렬로 만든다 (Scene::onUpdate 참고).
    //    여기 있는 건 투영(projection) 쪽뿐이다.
    struct CameraMatrixComponent {
        static constexpr const char* TypeName() { return "CameraMatrixComponent"; }

        SceneCamera sceneCam;
        bool primary = false;
        bool fixed_aspectRatio = false;
        bool defaultCam = false;

        CameraMatrixComponent() = default;
        CameraMatrixComponent(const CameraMatrixComponent&) = default;
    };

}

#pragma once
#include <string>
#include "renderer/camera/sceneCamera.h"

namespace AppWMs {

    // "How does it see" -- the component that makes this entity a camera.
    //
    // WARNING: the view matrix is not here. A camera uses the same TransformComponent as any
    //    other object, and the view is built from its inverse (see Scene::onUpdate).
    //    What lives here is only the projection side.
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

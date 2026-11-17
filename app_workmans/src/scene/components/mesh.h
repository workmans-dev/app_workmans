#pragma once
#include "core/core.h"              // Ref
#include "renderer/vertexArray.h"

namespace AppWMs {

    // "What shape is it" -- a polygon mesh.
    //
    // The entity owns its own mesh. If the scene recycled one shared copy, two objects
    // could not be edited differently -- a premise that does not hold in a modeling app.
    //
    // WARNING: primitives (cube/plane/sphere) are not a kind of component.
    //    A "cube" is only a factory that stamps out an initial shape (renderer/geometry/cube.h);
    //    once made, it is just an editable polygon mesh. Same treatment as Blender.
    //    (Maya layers construction history on top so the parameters can be touched again,
    //     but that is a separate axis called parametric history, not a kind of shape.)
    //
    // For now it holds the GPU-side VertexArray directly, but once a CPU-side editable Mesh
    // type exists this becomes Ref<Mesh>. The shape of the component stays the same.
    struct MeshComponent {
        static constexpr const char* TypeName() { return "MeshComponent"; }

        Ref<VertexArray> mesh;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
        MeshComponent(const Ref<VertexArray>& mesh) : mesh(mesh) {}
    };

}

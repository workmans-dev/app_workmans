#pragma once
#include "core/core.h"
#include "renderer/vertexArray.h"

namespace AppWMs {

    class Cube {
    public:
        // Creates a 24-vertex (independent per face) / 36-index cube VertexArray
        static Ref<VertexArray> create();

        // The single source of truth for the vertex layout.
        //    A pipeline needs only the layout, with no VertexArray instance, so it is pulled out separately.
        //    create() uses this too, so there is no room to diverge from the shader locations.
        static BufferLayout layout();

        static constexpr uint32_t vertexCount = 24;
        static constexpr uint32_t indexCount = 36;
    };
}
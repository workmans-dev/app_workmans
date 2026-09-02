#include "pch.h"
#include "renderer/geometry/cube.h"
#include "renderer/buffer.h"

namespace AppWMs {

    BufferLayout Cube::layout() {
        return {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal"   },
            { ShaderDataType::Float3, "a_Color"    }
        };
    }

    Ref<VertexArray> Cube::create() {

        // pos(3) / normal(3) / color(3) — 면당 4정점, 바깥에서 볼 때 CCW 감기
        static float s_Vertices[] = {
            // +Z (front) — red
            -0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.9f, 0.2f, 0.2f,
             0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.9f, 0.2f, 0.2f,
             0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.9f, 0.2f, 0.2f,
            -0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.9f, 0.2f, 0.2f,

            // -Z (back) — green
             0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.2f, 0.8f, 0.3f,
            -0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.2f, 0.8f, 0.3f,
            -0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.2f, 0.8f, 0.3f,
             0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.2f, 0.8f, 0.3f,

             // -X (left) — blue
             -0.5f,-0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   0.25f,0.45f,0.9f,
             -0.5f,-0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   0.25f,0.45f,0.9f,
             -0.5f, 0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   0.25f,0.45f,0.9f,
             -0.5f, 0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   0.25f,0.45f,0.9f,

             // +X (right) — yellow
              0.5f,-0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   0.95f,0.85f,0.2f,
              0.5f,-0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   0.95f,0.85f,0.2f,
              0.5f, 0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   0.95f,0.85f,0.2f,
              0.5f, 0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   0.95f,0.85f,0.2f,

              // +Y (top) — cyan
              -0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   0.3f, 0.85f,0.85f,
               0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   0.3f, 0.85f,0.85f,
               0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   0.3f, 0.85f,0.85f,
              -0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   0.3f, 0.85f,0.85f,

              // -Y (bottom) — magenta
              -0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   0.85f,0.3f, 0.8f,
               0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   0.85f,0.3f, 0.8f,
               0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   0.85f,0.3f, 0.8f,
              -0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   0.85f,0.3f, 0.8f,
        };

        // 면당 0,1,2 / 2,3,0 — 4씩 오프셋
        static uint32_t s_Indices[] = {
             0, 1, 2,   2, 3, 0,
             4, 5, 6,   6, 7, 4,
             8, 9,10,  10,11, 8,
            12,13,14,  14,15,12,
            16,17,18,  18,19,16,
            20,21,22,  22,23,20,
        };

        // vertexArray
        Ref<VertexArray>  va = VertexArray::create();

        // buffer
        Ref<VertexBuffer> vb = VertexBuffer::create(s_Vertices, sizeof(s_Vertices));
        vb->setLayout(layout());
        va->addVertexBuffer(vb);

        Ref<IndexBuffer> ib = IndexBuffer::create(s_Indices, Cube::indexCount);
        va->setIndexBuffer(ib);

        return va;
    }
}
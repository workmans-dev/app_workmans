#pragma once
#include "core/core.h"
#include "renderer/vertexArray.h"

namespace AppWMs {

    class Cube {
    public:
        // 🟢 24정점(면당 독립) / 36인덱스 큐브 VertexArray 생성
        static Ref<VertexArray> create();

        // 🟢 정점 레이아웃의 단일 원본.
        //    파이프라인은 VertexArray 인스턴스 없이 레이아웃만 필요하므로 따로 뽑아둔다.
        //    create()도 이걸 쓰므로 셰이더 location과 어긋날 여지가 없다.
        static BufferLayout layout();

        static constexpr uint32_t vertexCount = 24;
        static constexpr uint32_t indexCount = 36;
    };
}
#pragma once
#include "renderer/vertexArray.h"
#include <vulkan/vulkan.h>

namespace AppWMs {

    // BufferLayout(백엔드 중립) → 벌칸 정점 입력 기술자 변환.
    // 파이프라인 생성에만 쓰이므로 VertexArray 인스턴스 없이 레이아웃만으로 동작한다.
    namespace VulkanVertexInput {

        VkFormat ToVkFormat(ShaderDataType type);

        struct Description {
            std::vector<VkVertexInputBindingDescription>   bindings;
            std::vector<VkVertexInputAttributeDescription> attributes;
        };

        // BufferLayout 1개 = 바인딩 1개 (인터리브 버퍼).
        // location은 레이아웃 선언 순서와 일치한다 → cube.vert의 layout(location=N)과 대응.
        Description Build(const BufferLayout& layout, uint32_t binding = 0);
    }

    class VulkanVertexArray : public VertexArray {
    public:
        VulkanVertexArray() = default;
        virtual ~VulkanVertexArray() = default;

        // Vulkan은 전역 바인딩 상태가 없음 → no-op (실제 바인딩은 커맨드 기록 시점)
        virtual void bind()   const override {}
        virtual void unbind() const override {}

        virtual void addVertexBuffer(const Ref<VertexBuffer> vertexBuffer) override;
        virtual void setIndexBuffer(const Ref<IndexBuffer> indexBuffer) override;

        virtual const std::vector<Ref<VertexBuffer>> getVertexBuffers() const override { return m_VertexBuffers; }
        virtual const Ref<IndexBuffer> getIndexBuffer() const override { return m_IndexBuffer; }

        // 🟢 Vulkan 전용: 커맨드버퍼에 정점/인덱스 버퍼 바인딩 기록
        void bindToCommandBuffer(VkCommandBuffer cmd) const;
        bool hasIndexBuffer() const { return m_IndexBuffer != nullptr; }

    private:
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer>               m_IndexBuffer;

        // 🟢 핸들 캐시 (매 프레임 추출 방지, addVertexBuffer 시점에만 갱신)
        std::vector<VkBuffer>     m_VkBuffers;
        std::vector<VkDeviceSize> m_Offsets;
    };
}
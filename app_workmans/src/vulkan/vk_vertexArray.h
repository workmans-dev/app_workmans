#pragma once
#include "renderer/vertexArray.h"
#include <vulkan/vulkan.h>

namespace AppWMs {

    // Converts a BufferLayout (backend neutral) into Vulkan vertex input descriptors.
    // Only used when creating a pipeline, so it works from the layout alone, with no VertexArray instance.
    namespace VulkanVertexInput {

        VkFormat ToVkFormat(ShaderDataType type);

        struct Description {
            std::vector<VkVertexInputBindingDescription>   bindings;
            std::vector<VkVertexInputAttributeDescription> attributes;
        };

        // One BufferLayout = one binding (interleaved buffer).
        // location follows the declaration order of the layout -> matches layout(location=N) in cube.vert.
        Description Build(const BufferLayout& layout, uint32_t binding = 0);
    }

    class VulkanVertexArray : public VertexArray {
    public:
        VulkanVertexArray() = default;
        virtual ~VulkanVertexArray() = default;

        // Vulkan has no global binding state -> no-op (the real binding happens while recording commands)
        virtual void bind()   const override {}
        virtual void unbind() const override {}

        virtual void addVertexBuffer(const Ref<VertexBuffer> vertexBuffer) override;
        virtual void setIndexBuffer(const Ref<IndexBuffer> indexBuffer) override;

        virtual const std::vector<Ref<VertexBuffer>> getVertexBuffers() const override { return m_VertexBuffers; }
        virtual const Ref<IndexBuffer> getIndexBuffer() const override { return m_IndexBuffer; }

        // Vulkan only: records the vertex/index buffer bindings into the command buffer
        void bindToCommandBuffer(VkCommandBuffer cmd) const;
        bool hasIndexBuffer() const { return m_IndexBuffer != nullptr; }

    private:
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer>               m_IndexBuffer;

        // Handle cache (avoids re-extracting every frame; refreshed only in addVertexBuffer)
        std::vector<VkBuffer>     m_VkBuffers;
        std::vector<VkDeviceSize> m_Offsets;
    };
}

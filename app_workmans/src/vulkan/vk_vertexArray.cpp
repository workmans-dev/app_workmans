#include "pch.h"
#include "vulkan/vk_vertexArray.h"
#include "vulkan/vk_buffer.h"

namespace AppWMs {

    // ─────────────────────────────────────────────
    // VulkanVertexInput — BufferLayout → 벌칸 정점 입력 기술자
    // ─────────────────────────────────────────────
    namespace VulkanVertexInput {

        VkFormat ToVkFormat(ShaderDataType type) {
            switch (type) {
            case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
            case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
            case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
            case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case ShaderDataType::Int:    return VK_FORMAT_R32_SINT;
            case ShaderDataType::Int2:   return VK_FORMAT_R32G32_SINT;
            case ShaderDataType::Int3:   return VK_FORMAT_R32G32B32_SINT;
            case ShaderDataType::Int4:   return VK_FORMAT_R32G32B32A32_SINT;
            case ShaderDataType::Bool:   return VK_FORMAT_R8_UINT;

            // Mat3/Mat4는 정점 속성으로 쓰면 location을 3~4개 점유한다.
            // 어트리뷰트 1개 = location 1개인 현재 변환 규칙으로는 표현 불가.
            // 인스턴싱 도입 시 별도 경로로 처리한다.
            case ShaderDataType::Mat3:
            case ShaderDataType::Mat4:
                APP_WMS_CORE_ASSERTS(false, "Mat3/Mat4 정점 속성 미지원 (location 다중 점유)");
                return VK_FORMAT_UNDEFINED;
            }
            APP_WMS_CORE_ASSERTS(false, "Unknown ShaderDataType!");
            return VK_FORMAT_UNDEFINED;
        }

        Description Build(const BufferLayout& layout, uint32_t binding) {
            Description desc;

            // stride는 BufferLayout이 calculateOffsetAndStride에서 이미 누적해둔 값
            VkVertexInputBindingDescription bindingDesc{};
            bindingDesc.binding = binding;
            bindingDesc.stride = layout.getStride();
            bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            desc.bindings.push_back(bindingDesc);

            uint32_t location = 0;
            for (const auto& element : layout) {
                VkVertexInputAttributeDescription attr{};
                attr.location = location++;
                attr.binding = binding;
                attr.format = ToVkFormat(element.shaderDataType);
                attr.offset = element.offset;   // 레이아웃이 계산해둔 바이트 오프셋
                desc.attributes.push_back(attr);
            }
            return desc;
        }
    }

    void VulkanVertexArray::addVertexBuffer(const Ref<VertexBuffer> vertexBuffer) {
        APP_WMS_CORE_ASSERTS(vertexBuffer->getLayout().getElements().size(),
            "VertexBuffer has no layout!")

            m_VertexBuffers.push_back(vertexBuffer);

        // 추상 포인터에서 실제 VkBuffer 핸들만 뽑아 캐시에 적재
        auto vkVB = std::static_pointer_cast<VulkanVertexBuffer>(vertexBuffer);
        m_VkBuffers.push_back(vkVB->getHandle());
        m_Offsets.push_back(0);
    }

    void VulkanVertexArray::setIndexBuffer(const Ref<IndexBuffer> indexBuffer) {
        m_IndexBuffer = indexBuffer;
    }

    void VulkanVertexArray::bindToCommandBuffer(VkCommandBuffer cmd) const {
        if (!m_VkBuffers.empty()) {
            vkCmdBindVertexBuffers(cmd, 0, (uint32_t)m_VkBuffers.size(),
                m_VkBuffers.data(), m_Offsets.data());
        }
        if (m_IndexBuffer) {
            auto vkIB = std::static_pointer_cast<VulkanIndexBuffer>(m_IndexBuffer);
            // IndexBuffer::create가 uint32_t* 를 받으므로 UINT32 고정
            vkCmdBindIndexBuffer(cmd, vkIB->getHandle(), 0, VK_INDEX_TYPE_UINT32);
        }
    }
}
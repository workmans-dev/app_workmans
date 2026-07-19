#pragma once
#include "core/core.h"
#include "renderer/rendererAPI.h"
#include <vulkan/vulkan.h>

namespace AppWMs {

    class VulkanRendererAPI : public RendererAPI {
    public:
        virtual void init() override;
        virtual void set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
        virtual void setClearColor(const glm::vec4& color) override;
        virtual void clear() override;
        virtual void drawInexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
        virtual void drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0) override;
        virtual void setLineWidth(float width) override;

        // 🟢 외부(ImGui 등)에서 렌더패스와 풀을 빌려갈 수 있도록 게터 제공
        VkRenderPass getRenderPass() const { return m_RenderPass; }
        VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }

    private:
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        VkDescriptorPool     m_DescriptorPool = VK_NULL_HANDLE;
        glm::vec4 m_ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
    };

}
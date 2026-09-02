#pragma once
#include "core/core.h"
#include "renderer/rendererAPI.h"
#include <vulkan/vulkan.h>

namespace AppWMs {

    class VulkanPipeline;

    class VulkanRendererAPI : public RendererAPI {
    public:
        // for sub classes
        virtual void init() override;
        virtual bool beginFrame() override;
        virtual void endFrame() override;
        virtual void set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
        virtual void setClearColor(const glm::vec4& color) override;
        virtual void clear() override;
        virtual void drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
        virtual void drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0) override;
        virtual void setLineWidth(float width) override;
        virtual void pushObjectMatrices(const glm::mat4& mvp, const glm::mat4& model) override;
        virtual void waitIdle() override;
        virtual void shutdown() override;

        // 🟢 push constant는 VkPipelineLayout이 있어야 쓸 수 있는데 추상 API로는
        //    벌칸 타입을 넘길 수 없다. 그래서 VulkanPipeline::bind()가 자신을
        //    여기에 등록하고, pushObjectMatrices가 그 레이아웃을 빌려 쓴다.
        void setCurrentPipeline(const VulkanPipeline* pipeline) { m_CurrentPipeline = pipeline; }

        VkRenderPass getRenderPass() const { return m_RenderPass; }
        VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }

        // 🟢 프레임버퍼 (RenderPass와 한 쌍이므로 같은 클래스가 소유)
        //    리사이즈 시 스왑체인 재생성 '이후'에 호출할 것
        void recreateFramebuffers();
        VkFramebuffer getFramebuffer(uint32_t index) const { return m_vkFramebuffers[index]; }
        uint32_t      getFramebufferCount() const { return (uint32_t)m_vkFramebuffers.size(); }
        const glm::vec4& getClearColor() const { return m_ClearColor; }
    private:
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        VkDescriptorPool     m_DescriptorPool = VK_NULL_HANDLE;
        // ⚠️ 이름 주의 — 우리 FrameBuffer 클래스가 아니라 벌칸 VkFramebuffer 핸들이다.
        //    여기 있는 건 전부 스왑체인용(화면 출력). 뷰포트 패널용 오프스크린은
        //    VulkanRenderTarget이 따로 소유한다.
        std::vector<VkFramebuffer> m_vkFramebuffers;
        glm::vec4 m_ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        const VulkanPipeline* m_CurrentPipeline = nullptr;   // 소유 아님 (빌려 쓰는 참조)
    };

}
#pragma once
#include "renderer/renderTarget.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace AppWMs {

    // 오프스크린 렌더타겟. 스왑체인이 아닌 자체 이미지에 그리고,
    // 그 결과를 ImGui가 텍스처로 샘플링해 뷰포트 패널에 붙인다.
    //
    // ② 결정 A안 — VkRenderPass를 이 클래스가 멤버로 소유한다 (타겟마다 1개).
    //   근거: 렌더패스는 어태치먼트 구성을 박아넣은 객체라, 공유하면 구성이
    //         강제로 같아진다. 피킹 도입 시 뷰포트 타겟(RGBA8+R32_SINT+Depth)과
    //         프리뷰 타겟(RGBA8)으로 구성이 갈리므로 공유가 성립하지 않는다.
    class VulkanRenderTarget : public RenderTarget {
    public:
        VulkanRenderTarget(const RenderTargetSpec& spec);
        virtual ~VulkanRenderTarget();

        virtual void beginPass() override;  // vkCmdBeginRenderPass
        virtual void endPass() override;    // vkCmdEndRenderPass
        virtual void resize(uint32_t w, uint32_t h) override;

        virtual const RenderTargetSpec& get_spec() const override { return m_Spec; }

        // ImGui::Image()에 그대로 넘길 핸들. 벌칸에선 VkDescriptorSet.
        virtual uint64_t getColorTextureID(int index = 0) const override {
            return (uint64_t)m_DescriptorSet;
        }

        // 파이프라인(3단계)이 대상 렌더패스를 빌려가는 창구
        VkRenderPass getRenderPass() const { return m_RenderPass; }

    private:
        void createRenderPass();  // 생성자에서 1회. 크기와 무관하므로 resize에서 재생성 안 함
        void invalidate();        // 이미지/뷰/프레임버퍼/샘플러/디스크립터셋 생성
        void cleanup();           // 위 것들만 파괴 (렌더패스 제외)

    private:
        RenderTargetSpec m_Spec;

        // 색 어태치먼트 — COLOR_ATTACHMENT(그리기) + SAMPLED(ImGui가 읽기)
        VkImage       m_ColorImage = VK_NULL_HANDLE;
        VmaAllocation m_ColorAlloc = nullptr;
        VkImageView   m_ColorView = VK_NULL_HANDLE;
        VkFormat      m_ColorFormat = VK_FORMAT_R8G8B8A8_UNORM;

        // 깊이 어태치먼트 — spec.attachments에 Depth가 있을 때만
        VkImage       m_DepthImage = VK_NULL_HANDLE;
        VmaAllocation m_DepthAlloc = nullptr;
        VkImageView   m_DepthView = VK_NULL_HANDLE;
        VkFormat      m_DepthFormat = VK_FORMAT_UNDEFINED;
        bool          m_HasDepth = false;

        // ② A안 — 타겟마다 1개씩 소유
        VkRenderPass  m_RenderPass = VK_NULL_HANDLE;
        // ⚠️ 이건 벌칸 API 핸들(슬롯 배선표)이지 이 클래스와 같은 개념이 아니다.
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

        // ImGui 샘플링 경로
        VkSampler       m_Sampler = VK_NULL_HANDLE;
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    };
}

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
        // index는 색 어태치먼트 번호다(깊이는 안 센다). 샘플링 대상이 아닌
        // 슬롯(R32_SINT)은 디스크립터셋이 없어 0을 돌려준다.
        virtual uint64_t getColorTextureID(int index = 0) const override {
            if (index < 0 || index >= (int)m_Colors.size()) return 0;
            return (uint64_t)m_Colors[index].descriptorSet;
        }

        // 파이프라인(3단계)이 대상 렌더패스를 빌려가는 창구
        VkRenderPass getRenderPass() const { return m_RenderPass; }

        // ⚠️ 파이프라인의 컬러블렌드 어태치먼트 개수는 subpass의
        //    colorAttachmentCount와 반드시 같아야 한다. 다르면 파이프라인 생성 거부.
        uint32_t getColorAttachmentCount() const { return (uint32_t)m_Colors.size(); }

        virtual void requestPixelRead(int attachmentIndex, uint32_t x, uint32_t y) override;
        virtual bool pollPixelRead(int32_t& outValue) override;

    private:
        void createRenderPass();  // 생성자에서 1회. 크기와 무관하므로 resize에서 재생성 안 함
        void invalidate();        // 이미지/뷰/프레임버퍼/샘플러/디스크립터셋 생성
        void cleanup();           // 위 것들만 파괴 (렌더패스 제외)

        // 픽셀 리드백 — 스테이징 버퍼는 4바이트짜리라 크기와 무관하다.
        // 그래서 invalidate/cleanup이 아니라 생성자/소멸자가 잡는다.
        void createReadbackBuffers();
        void destroyReadbackBuffers();
        // endPass()가 매 프레임 부른다: 익은 것 수확 → 새 요청 기록
        void harvestAndRecordPixelRead(VkCommandBuffer cmd);

    private:
        RenderTargetSpec m_Spec;

        // 색 어태치먼트 N개. 나열 순서가 곧 렌더패스 슬롯 번호이자
        // 프래그먼트 셰이더의 out location이다.
        //   슬롯 0: RGBA8    — 그리기 + ImGui 샘플링
        //   슬롯 1: R32_SINT — 그리기 + CPU 리드백 (P-b). 샘플링 안 함
        struct ColorAttachment {
            VkImage         image = VK_NULL_HANDLE;
            VmaAllocation   alloc = nullptr;
            VkImageView     view = VK_NULL_HANDLE;
            VkFormat        format = VK_FORMAT_UNDEFINED;
            // sampled=false면 SAMPLED usage도, 디스크립터셋도, 최초 레이아웃
            // 전환도 만들지 않는다. 정수 포맷은 선형 필터링 자체가 안 된다.
            bool            sampled = false;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        };
        std::vector<ColorAttachment> m_Colors;

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

        // ImGui 샘플링 경로. 샘플러는 sampled 슬롯들이 공유한다.
        VkSampler       m_Sampler = VK_NULL_HANDLE;

        // ── 픽셀 리드백 (P-b) ────────────────────────────────────────────────
        // ⚠️ frame-in-flight 슬롯마다 하나씩 두는 게 핵심이다. 슬롯 S에 복사를
        //    기록하면, 2프레임 뒤 그 슬롯으로 돌아왔을 때 beginFrame이 이미
        //    fence[S]를 기다린 뒤라 **복사 완료가 공짜로 보장된다.**
        //    버퍼 하나를 돌려쓰면 그 보장이 없어 vkDeviceWaitIdle이 필요해진다.
        struct Readback {
            VkBuffer      buffer = VK_NULL_HANDLE;
            VmaAllocation alloc = nullptr;
            void* mapped = nullptr;   // 상주 매핑 (4B라 매번 map/unmap할 이유가 없다)
            bool          pending = false;     // 이 슬롯에 수확 대기 중인 복사가 있나
        };
        std::vector<Readback> m_Readbacks;   // [frame-in-flight 슬롯]

        // 다음 endPass()에서 기록할 요청. -1이면 요청 없음.
        int      m_PendingReadAttachment = -1;
        uint32_t m_PendingReadX = 0, m_PendingReadY = 0;

        // 수확 결과. pollPixelRead가 꺼내가면 다시 false가 된다.
        bool     m_HasResult = false;
        int32_t  m_Result = -1;
    };
}

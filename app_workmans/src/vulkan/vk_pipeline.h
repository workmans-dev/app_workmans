#pragma once
#include "renderer/buffer.h"
#include <vulkan/vulkan.h>

namespace AppWMs {

    // ④ 결정 C안 — 파이프라인을 독립 클래스로 둔다.
    //
    // 파이프라인은 셰이더 하나로 결정되지 않는다. 아래 조합 전체가 파이프라인의 정체다:
    //   셰이더 + 렌더패스 + 정점레이아웃 + 렌더상태
    // 같은 셰이더라도 렌더패스나 상태가 다르면 별개 VkPipeline이어야 한다.
    //
    // 이 구조의 목적은 속도가 아니라 정렬 가능성이다. 드로우를 파이프라인별로
    // 묶어 상태 전환을 줄이려면 파이프라인이 밖에서 비교 가능한 키여야 한다.
    struct PipelineSpec {
        // 빌드 시 compile_shaders.bat이 구운 .spv 경로 (GLSL 원본 아님)
        std::string  vertPath;
        std::string  fragPath;

        VkRenderPass renderPass = VK_NULL_HANDLE;   // 어느 FB에 그리나
        BufferLayout vertexLayout;                  // 정점이 어떻게 생겼나
        uint32_t     pushConstantSize = 0;          // VERTEX 스테이지 기준, 0이면 미사용

        // 렌더 상태 — 여기가 다르면 셰이더가 같아도 별개 파이프라인
        VkPolygonMode   polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;

        // ⚠️ CCW가 기본 — 판정 근거는 cube.frag의 조명이다.
        //    양면 조명이 없으므로 앞면(법선이 바깥)을 보면 diff > 0 이라 밝고,
        //    뒷면이 드러나면 법선이 카메라 반대라 diff ≈ 0 → ambient(0.25)만 남아 어둡다.
        //    즉 "어두워지면 앞면을 잘라먹고 있다"는 신호다.
        //    조건: cube.cpp의 모델 공간 CCW 감기 + Scene의 proj[1][1] *= -1 조합.
        //
        // ⚠️ 음수 스케일(미러링) 주의 — 축 하나를 -1로 주면 그 오브젝트만
        //    감기가 뒤집혀 컬링이 반대로 동작한다. 모델링 앱에선 흔한 경우라
        //    미러 기능을 붙일 때 별도 처리가 필요하다.
        //    (그래서 scale.y = -1은 컬링 진단 도구로 부적절하다 — 감기만 바꾸는 게
        //     아니라 형상 자체를 거울반전시켜 보이는 면이 통째로 달라진다.)
        VkFrontFace     frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        bool            depthTest = true;
        bool            depthWrite = true;
    };

    class VulkanPipeline {
    public:
        VulkanPipeline(const PipelineSpec& spec);
        ~VulkanPipeline();

        // VkPipeline은 소유 핸들이라 복사되면 이중 파괴가 난다
        VulkanPipeline(const VulkanPipeline&) = delete;
        VulkanPipeline& operator=(const VulkanPipeline&) = delete;

        void bind(VkCommandBuffer cmd) const;
        void pushConstants(VkCommandBuffer cmd, const void* data, uint32_t size) const;

        bool isValid() const { return m_Pipeline != VK_NULL_HANDLE; }
        VkPipeline       getHandle() const { return m_Pipeline; }
        VkPipelineLayout getLayout() const { return m_Layout; }

    private:
        VkPipeline       m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
        uint32_t         m_PushConstantSize = 0;
    };
}

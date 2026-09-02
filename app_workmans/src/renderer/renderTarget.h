#pragma once

#include "core/core.h"
#include <vector>

namespace AppWMs {

    // 렌더타겟에 붙는 이미지 한 장의 용도.
    enum class AttachmentFormat {
        None = 0,
        RGBA8,          // 색
        DEPTHSTENCIL8,  // 깊이(+스텐실)
        Depth = DEPTHSTENCIL8
    };

    struct RenderTargetSpec {
        uint32_t w = 0, h = 0;
        uint32_t samples = 1;
        // { RGBA8, Depth } 처럼 나열한다. 나열 순서가 곧 어태치먼트 슬롯 번호.
        std::vector<AttachmentFormat> attachments;
    };

    // "어디에 그리나" — 화면이 아닌 자체 이미지에 그리는 대상.
    //
    // ⚠️ 예전 이름은 FrameBuffer였다. OpenGL FBO 시절 용어라 역할과 안 맞았다:
    //    VkFramebuffer는 이 클래스가 가진 부품 하나일 뿐이고, 픽셀을 담지도 않는다.
    //    이 클래스의 정체는 "그려질 이미지 + 그 결과를 텍스처로 읽어가는 경로"를
    //    묶어서 소유하는 렌더 대상이다. D3D/Unity/UE가 쓰는 render target과 같은 개념.
    class RenderTarget {
    public:
        static Ref<RenderTarget> create(const RenderTargetSpec& spec);

        virtual ~RenderTarget() = default;

        // ⚠️ bind/unbind가 아니다. 벌칸엔 "현재 바인딩된 대상"이라는 전역 상태가 없다.
        //    실제로 하는 일은 커맨드버퍼에 렌더패스를 열고 닫는 것이고,
        //    명령은 이 객체가 아니라 커맨드버퍼에 쌓인다.
        //    커맨드버퍼는 구현체가 컨텍스트에서 빌려 쓰므로 인자가 없다.
        virtual void beginPass() = 0;
        virtual void endPass() = 0;

        virtual void resize(uint32_t w, uint32_t h) = 0;

        virtual const RenderTargetSpec& get_spec() const = 0;

        // ImGui::Image()에 그대로 넘길 텍스처 핸들.
        // 벌칸은 VkDescriptorSet(64비트) → ImTextureID(ImU64)와 폭을 맞춘다.
        virtual uint64_t getColorTextureID(int index = 0) const = 0;
    };
}

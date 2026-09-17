#include "pch.h"
#include "vulkanRenderer.h"
#include "vulkan/vulkanContext.h" // 장치 정보 접근용
#include "vulkan/vk_vertexArray.h" // bindToCommandBuffer
#include "vulkan/vk_pipeline.h"    // pushConstants

namespace AppWMs {

    void VulkanRendererAPI::init() {
        APP_WMS_CORE_INFO("VulkanRendererAPI: RenderPass & DescriptorPool !");

        VkDevice device = VulkanContext::Get()->getDevice();
        VkFormat swapChainImageFormat = VulkanContext::Get()->getSwapChainImageFormat();

        // =========================================================================
        // 🎨 1. RenderPass (도화지 세팅) 조립
        // =========================================================================
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // 그리기 전에 화면 지움
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 그린 후 화면 보존
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 최종 화면 송출용 레이아웃

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ⚠️ 스왑체인 패스는 ImGui 전용 → 깊이 어태치먼트 없음.
        //    3D 깊이 테스트는 오프스크린 RenderPass가 담당한다.

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = nullptr;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;

        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("RenderPass 생성 실패!");
        }

        // =========================================================================
        // 🧺 2. DescriptorPool (물감 바구니) 조립
        // =========================================================================
        // ImGui 및 엔진 텍스처들이 넉넉하게 쓸 수 있도록 크기를 크게 잡아줍니다.
        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * 11; // 넉넉한 최대 세트 수
        pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_info.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(device, &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("DescriptorPool 생성 실패!");
        }
        recreateFramebuffers();
        APP_WMS_CORE_INFO("VulkanRendererAPI: RenderPass & DescriptorPool 생성 완료!");
    }
    void VulkanRendererAPI::recreateFramebuffers() {
        auto ctx = VulkanContext::Get();
        VkDevice device = ctx->getDevice();

        // 기존 것 파괴 (리사이즈 재호출 대비 — 최초 호출 시엔 비어 있어 no-op)
        for (auto fb : m_vkFramebuffers) {
            if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(device, fb, nullptr);
        }
        m_vkFramebuffers.clear();

        const auto& colorViews = ctx->getSwapChainImageViews();
        VkExtent2D  extent = ctx->getSwapChainExtent();

        m_vkFramebuffers.resize(colorViews.size());

        for (size_t i = 0; i < colorViews.size(); i++) {
            // ⭐️ 배열 순서 = RenderPass의 attachment 인덱스 (0=color)
            VkImageView attachments[] = { colorViews[i] };

            VkFramebufferCreateInfo fbInfo{};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = m_RenderPass;
            fbInfo.attachmentCount = 1;
            fbInfo.pAttachments = attachments;
            fbInfo.width = extent.width;
            fbInfo.height = extent.height;
            fbInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbInfo, nullptr, &m_vkFramebuffers[i]) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("Framebuffer 생성 실패! (index {0})", i);
            }
        }

        APP_WMS_CORE_INFO("Framebuffer {0}개 생성 ({1}x{2})",
            m_vkFramebuffers.size(), extent.width, extent.height);
    }

    bool VulkanRendererAPI::beginFrame() {
        auto ctx = VulkanContext::Get();

        // 🟢 스왑체인 재생성은 여기서만 한다. 커맨드버퍼가 아직 열리기 전이라 안전하다.
        //    Context가 스스로 하지 않는 이유: 프레임버퍼는 상위(이 클래스)가 소유하므로
        //    재생성 순서를 아는 쪽이 여기다. 하위가 상위를 역참조하지 않는 원칙도 유지된다.
        if (ctx->isSwapChainDirty()) {
            if (!ctx->recreateSwapChain()) {
                // 최소화 등으로 아직 못 만든다 → 낡음 표시를 유지한 채 이번 프레임 스킵
                return false;
            }
            // 스왑체인 이미지뷰가 통째로 바뀌었으니 그걸 붙들고 있던 FB도 다시 만든다
            recreateFramebuffers();
        }

        // Context가 acquire + 커맨드버퍼 개시. out-of-date면 false → 프레임 스킵
        return ctx->beginFrame();
    }

    void VulkanRendererAPI::endFrame() {
        // Context가 커맨드버퍼 종료 + submit + present + 슬롯 회전
        VulkanContext::Get()->swapBuffers();
    }


    // ─────────────────────────────────────────────────────────────
    // 🚧 스텁 — vtable 구성용. 해당 단계에서 채운다.
    // ─────────────────────────────────────────────────────────────
    void VulkanRendererAPI::set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
        // 창 크기가 바뀌면 스왑체인이 낡는다. 여기선 표시만 남기고
        // 실제 재생성은 다음 beginFrame(프레임 경계)이 처리한다.
        // ⚠️ 이 이벤트는 프레임 한복판(커맨드버퍼 기록 중)에도 들어올 수 있다.
        VulkanContext::Get()->requestSwapChainRecreate();
    }

    void VulkanRendererAPI::setClearColor(const glm::vec4& color) {
        m_ClearColor = color;
    }

    void VulkanRendererAPI::clear() {
        // 벌칸은 RenderPass loadOp=CLEAR가 담당 → no-op
    }

    void VulkanRendererAPI::drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {
        if (!vertexArray) return;

        // ⚠️ 전제: 호출자가 렌더패스를 열고(FrameBuffer::bind) 파이프라인을 바인딩한 상태여야 한다.
        //    벌칸은 전역 상태가 없어서 이 함수가 스스로 확인할 방법이 없다.
        auto vkVA = std::static_pointer_cast<VulkanVertexArray>(vertexArray);

        const Ref<IndexBuffer>& ib = vertexArray->getIndexBuffer();
        if (!ib) {
            APP_WMS_CORE_ERROR("drawIndexed: IndexBuffer가 없다");
            return;
        }

        VkCommandBuffer cmd = VulkanContext::Get()->getCurrentCommandBuffer();

        // 정점/인덱스 버퍼를 커맨드버퍼에 바인딩 (핸들은 addVertexBuffer 때 캐시해둔 것)
        vkVA->bindToCommandBuffer(cmd);

        // indexCount 0 = "전부 그린다" (기존 추상 시그니처의 기본값 규약)
        const uint32_t count = indexCount ? indexCount : ib->getCount();

        vkCmdDrawIndexed(cmd, count, 1, 0, 0, 0);
    }

    void VulkanRendererAPI::drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
        // 추후
    }

    void VulkanRendererAPI::setLineWidth(float width) {
        // 추후 (VK_DYNAMIC_STATE_LINE_WIDTH)
    }

    void VulkanRendererAPI::pushObjectMatrices(const glm::mat4& mvp, const glm::mat4& model,
        int32_t entityID) {
        if (!m_CurrentPipeline) {
            APP_WMS_CORE_ERROR("pushObjectMatrices: 바인딩된 파이프라인이 없다");
            return;
        }
        // ⚠️ cube.vert/frag의 Push 블록과 바이트 단위로 같아야 한다.
        //    어긋나면 컴파일도 validation도 통과하는데 화면만 조용히 깨진다.
        struct Push {
            glm::mat4 mvp;        //   0..63
            glm::mat4 model;      //  64..127
            int32_t   entityID;   // 128..131
        } data{ mvp, model, entityID };
        static_assert(sizeof(Push) == 132, "cube.vert/frag의 Push 블록과 크기가 어긋났다");

        m_CurrentPipeline->pushConstants(
            VulkanContext::Get()->getCurrentCommandBuffer(), &data, sizeof(data));
    }

    void VulkanRendererAPI::waitIdle() {
        vkDeviceWaitIdle(VulkanContext::Get()->getDevice());
    }

    void VulkanRendererAPI::shutdown() {
        // 전제: App이 waitIdle → 레이어 파괴 → ImGui 백엔드 종료까지 마친 뒤에 부른다.
        //       디스크립터풀은 ImGui가 쓰던 것이라 그보다 먼저 파괴하면 안 된다.
        VkDevice device = VulkanContext::Get()->getDevice();
        if (device == VK_NULL_HANDLE) return;

        for (VkFramebuffer fb : m_vkFramebuffers) {
            if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(device, fb, nullptr);
        }
        m_vkFramebuffers.clear();

        if (m_DescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
            m_DescriptorPool = VK_NULL_HANDLE;
        }
        if (m_RenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, m_RenderPass, nullptr);
            m_RenderPass = VK_NULL_HANDLE;
        }
        m_CurrentPipeline = nullptr;

        APP_WMS_CORE_INFO("VulkanRendererAPI: resources released");
    }
}
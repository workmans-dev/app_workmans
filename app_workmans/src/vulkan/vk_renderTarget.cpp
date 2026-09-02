#include "pch.h"
#include "vulkan/vk_renderTarget.h"
#include "vulkan/vulkanContext.h"
#include "vulkan/vk_swapChain.h"   // CreateImageView / FindDepthFormat 재사용
#include "vulkan/vk_transfer.h"    // BeginSingleTime / EndSingleTime 재사용
#include "vulkan/vulkanRenderer.h"
#include "renderer/rendererCommand.h"

#include "backends/imgui_impl_vulkan.h"

namespace AppWMs {

    VulkanRenderTarget::VulkanRenderTarget(const RenderTargetSpec& spec)
        : m_Spec(spec)
    {
        // 어태치먼트 해석 — 현재 지원: 색 1개 + 깊이 0~1개.
        // 피킹용 R32_SINT는 추상 열거자가 생긴 뒤에 추가한다.
        for (AttachmentFormat attach : m_Spec.attachments) {
            switch (attach) {
            case AttachmentFormat::RGBA8:
                m_ColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
                break;
            case AttachmentFormat::DEPTHSTENCIL8:
                m_HasDepth = true;
                break;
            default:
                break;
            }
        }

        if (m_HasDepth) {
            m_DepthFormat = VulkanSwapChain::FindDepthFormat(
                VulkanContext::Get()->getPhysicalDevice());
        }

        createRenderPass();  // 크기와 무관 → 여기서 딱 1회
        invalidate();
    }

    VulkanRenderTarget::~VulkanRenderTarget() {
        cleanup();
        VkDevice device = VulkanContext::Get()->getDevice();
        if (m_RenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, m_RenderPass, nullptr);
        }
    }

    // =========================================================================
    // RenderPass — 어태치먼트 구성 기술서. 크기를 담지 않으므로 리사이즈와 무관.
    // =========================================================================
    void VulkanRenderTarget::createRenderPass() {
        VkDevice device = VulkanContext::Get()->getDevice();

        std::vector<VkAttachmentDescription> attachments;
        attachments.reserve(2);

        // attachment 0 — 색.
        // finalLayout이 스왑체인 패스와 갈리는 지점: 여기선 present가 아니라
        // ImGui가 샘플링해야 하므로 SHADER_READ_ONLY_OPTIMAL로 끝낸다.
        VkAttachmentDescription color{};
        color.format = m_ColorFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;    // 샘플링해야 하므로 보존
        color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments.push_back(color);

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // attachment 1 — 깊이. 다음 프레임에 다시 CLEAR하므로 저장할 필요 없음.
        VkAttachmentReference depthRef{};
        if (m_HasDepth) {
            VkAttachmentDescription depth{};
            depth.format = m_DepthFormat;
            depth.samples = VK_SAMPLE_COUNT_1_BIT;
            depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments.push_back(depth);

            depthRef.attachment = 1;
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;
        subpass.pDepthStencilAttachment = m_HasDepth ? &depthRef : nullptr;

        // 의존성 2개 — 이 FB는 매 프레임 "그리기 → ImGui가 읽기"를 반복한다.
        //  [0] 이전 프레임 ImGui 샘플링이 끝난 뒤에 이번 프레임 쓰기를 시작
        //  [1] 이번 프레임 쓰기가 끝난 뒤에 ImGui 샘플링을 시작
        VkSubpassDependency deps[2]{};

        deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        deps[0].dstSubpass = 0;
        deps[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        deps[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        deps[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        deps[1].srcSubpass = 0;
        deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        deps[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        deps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        deps[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.attachmentCount = (uint32_t)attachments.size();
        rpInfo.pAttachments = attachments.data();
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpass;
        rpInfo.dependencyCount = 2;
        rpInfo.pDependencies = deps;

        if (vkCreateRenderPass(device, &rpInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("offscreeen renderPass creation failed!");
        }
    }

    // =========================================================================
    // 크기에 묶인 자원 — 리사이즈마다 파괴 후 재생성되는 것들
    // =========================================================================
    void VulkanRenderTarget::invalidate() {
        auto ctx = VulkanContext::Get();
        VkDevice     device = ctx->getDevice();
        VmaAllocator allocator = ctx->getAllocator();

        VmaAllocationCreateInfo allocCI{};
        allocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        // ── 색 이미지: 그리기 대상이면서 동시에 셰이더가 읽는 텍스처
        VkImageCreateInfo colorCI{};
        colorCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        colorCI.imageType = VK_IMAGE_TYPE_2D;
        colorCI.format = m_ColorFormat;
        colorCI.extent = { m_Spec.w, m_Spec.h, 1 };
        colorCI.mipLevels = 1;
        colorCI.arrayLayers = 1;
        colorCI.samples = VK_SAMPLE_COUNT_1_BIT;
        colorCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        colorCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT;
        colorCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        colorCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vmaCreateImage(allocator, &colorCI, &allocCI,
            &m_ColorImage, &m_ColorAlloc, nullptr) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("offscreen color image creation failed!");
            return;
        }
        m_ColorView = VulkanSwapChain::CreateImageView(device, m_ColorImage,
            m_ColorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

        // ── 깊이 이미지: 뷰포트 패널 크기로 자기 것을 가진다 (2-a에서 스왑체인에서 뗀 이유)
        if (m_HasDepth) {
            VkImageCreateInfo depthCI = colorCI;
            depthCI.format = m_DepthFormat;
            depthCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            if (vmaCreateImage(allocator, &depthCI, &allocCI,
                &m_DepthImage, &m_DepthAlloc, nullptr) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("offscreen depth image creation failed!");
                return;
            }
            m_DepthView = VulkanSwapChain::CreateImageView(device, m_DepthImage,
                m_DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        // ── 프레임버퍼: 위 뷰들을 렌더패스 어태치먼트 슬롯에 묶는다 (배열 순서 = 슬롯 번호)
        std::vector<VkImageView> views;
        views.push_back(m_ColorView);
        if (m_HasDepth) views.push_back(m_DepthView);

        VkFramebufferCreateInfo fbCI{};
        fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCI.renderPass = m_RenderPass;
        fbCI.attachmentCount = (uint32_t)views.size();
        fbCI.pAttachments = views.data();
        fbCI.width = m_Spec.w;
        fbCI.height = m_Spec.h;
        fbCI.layers = 1;

        if (vkCreateFramebuffer(device, &fbCI, nullptr, &m_Framebuffer) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("offscreen Framebuffer creation failed!");
            return;
        }

        // ── 샘플러: ImGui가 이 색 이미지를 텍스처로 읽을 때 쓴다
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.maxLod = 1.0f;
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if (vkCreateSampler(device, &samplerCI, nullptr, &m_Sampler) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("offscreen Sampler creation failed!");
            return;
        }

        // ── 최초 레이아웃 전환 (UNDEFINED → SHADER_READ_ONLY_OPTIMAL)
        // 렌더패스가 initialLayout=UNDEFINED라 그리기엔 문제없지만, bind() 없이
        // 샘플링되는 프레임이 한 번이라도 있으면 validation 에러가 난다.
        // 생성 시 1회 전환해두면 그 구멍이 막힌다.
        {
            VkCommandBuffer cmd = VulkanTransfer::BeginSingleTime(
                device, ctx->getGraphicsCommandPool());

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_ColorImage;
            barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier);

            VulkanTransfer::EndSingleTime(device, ctx->getGraphicsCommandPool(),
                ctx->getGraphicsQueue(), cmd);
        }

        // ── ImGui 디스크립터셋: 이 핸들이 곧 getColorTextureID()의 반환값
        m_DescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ColorView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        APP_WMS_CORE_INFO("offscreen RenderTarget {0}x{1} (depth: {2})",
            m_Spec.w, m_Spec.h, m_HasDepth);
    }

    void VulkanRenderTarget::cleanup() {
        VkDevice     device = VulkanContext::Get()->getDevice();
        VmaAllocator allocator = VulkanContext::Get()->getAllocator();

        // 디스크립터셋을 반납하지 않으면 리사이즈마다 DescriptorPool이 샌다
        if (m_DescriptorSet != VK_NULL_HANDLE) {
            ImGui_ImplVulkan_RemoveTexture(m_DescriptorSet);
            m_DescriptorSet = VK_NULL_HANDLE;
        }
        if (m_Sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, m_Sampler, nullptr);
            m_Sampler = VK_NULL_HANDLE;
        }
        if (m_Framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
            m_Framebuffer = VK_NULL_HANDLE;
        }
        if (m_DepthView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_DepthView, nullptr);
            m_DepthView = VK_NULL_HANDLE;
        }
        if (m_DepthImage != VK_NULL_HANDLE) {
            vmaDestroyImage(allocator, m_DepthImage, m_DepthAlloc);
            m_DepthImage = VK_NULL_HANDLE;
            m_DepthAlloc = nullptr;
        }
        if (m_ColorView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_ColorView, nullptr);
            m_ColorView = VK_NULL_HANDLE;
        }
        if (m_ColorImage != VK_NULL_HANDLE) {
            vmaDestroyImage(allocator, m_ColorImage, m_ColorAlloc);
            m_ColorImage = VK_NULL_HANDLE;
            m_ColorAlloc = nullptr;
        }
    }

    // =========================================================================
    void VulkanRenderTarget::beginPass() {
        auto ctx = VulkanContext::Get();
        VkCommandBuffer cmd = ctx->getCurrentCommandBuffer();

        // clear 색은 RendererCommand::setClearColor가 넣어둔 값을 쓴다.
        // 벌칸에서 clear()는 no-op이고 실제 지우기는 loadOp=CLEAR가 담당한다.
        const glm::vec4& cc =
            static_cast<VulkanRendererAPI*>(&RendererCommand::getAPI())->getClearColor();

        VkClearValue clears[2]{};
        clears[0].color = { { cc.r, cc.g, cc.b, cc.a } };
        clears[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo rpBegin{};
        rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.renderPass = m_RenderPass;
        rpBegin.framebuffer = m_Framebuffer;
        rpBegin.renderArea.offset = { 0, 0 };
        rpBegin.renderArea.extent = { m_Spec.w, m_Spec.h };
        rpBegin.clearValueCount = m_HasDepth ? 2u : 1u;
        rpBegin.pClearValues = clears;

        vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        // 뷰포트/시저는 동적 상태로 간다 — 파이프라인을 패널 크기마다
        // 다시 만들지 않기 위해서. 3단계에서 DYNAMIC_STATE로 선언한다.
        VkViewport viewport{ 0.0f, 0.0f, (float)m_Spec.w, (float)m_Spec.h, 0.0f, 1.0f };
        VkRect2D   scissor{ { 0, 0 }, { m_Spec.w, m_Spec.h } };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void VulkanRenderTarget::endPass() {
        vkCmdEndRenderPass(VulkanContext::Get()->getCurrentCommandBuffer());
    }

    void VulkanRenderTarget::resize(uint32_t w, uint32_t h) {
        if (w == 0 || h == 0) return;
        if (w == m_Spec.w && h == m_Spec.h) return;  // 매 프레임 호출되므로 같은 크기는 무시

        m_Spec.w = w;
        m_Spec.h = h;

        // ③ 결정 A안 — GPU가 옛 이미지를 다 쓸 때까지 기다린 뒤 파괴한다.
        // OpenGL은 드라이버가 지연 삭제를 대신 해줬지만 벌칸은 앱 책임.
        vkDeviceWaitIdle(VulkanContext::Get()->getDevice());

        cleanup();
        invalidate();  // ② 운용 규칙: 렌더패스는 유지 (크기와 무관)
    }
}

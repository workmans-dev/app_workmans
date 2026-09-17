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
        // 어태치먼트 해석 — 색 N개 + 깊이 0~1개.
        // 나열 순서가 그대로 렌더패스 슬롯 번호이자 프래그먼트 셰이더의 out location이다.
        for (AttachmentFormat attach : m_Spec.attachments) {
            switch (attach) {
            case AttachmentFormat::RGBA8: {
                ColorAttachment c;
                c.format = VK_FORMAT_R8G8B8A8_UNORM;
                c.sampled = true;    // ImGui가 뷰포트 패널에 붙여 보여준다
                m_Colors.push_back(c);
                break;
            }
            case AttachmentFormat::R32SINT: {
                ColorAttachment c;
                // R32_SINT는 벌칸 필수 지원 포맷(COLOR_ATTACHMENT)이라 가용성 조회가 필요 없다.
                c.format = VK_FORMAT_R32_SINT;
                c.sampled = false;   // 읽는 쪽이 셰이더가 아니라 CPU다 (P-b 리드백)
                m_Colors.push_back(c);
                break;
            }
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

        createRenderPass();       // 크기와 무관 → 여기서 딱 1회
        createReadbackBuffers();  // 4B짜리라 이것도 크기와 무관
        invalidate();
    }

    VulkanRenderTarget::~VulkanRenderTarget() {
        cleanup();
        destroyReadbackBuffers();
        VkDevice device = VulkanContext::Get()->getDevice();
        if (m_RenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, m_RenderPass, nullptr);
        }
    }

    // =========================================================================
    // 픽셀 리드백 — 스테이징 버퍼
    // =========================================================================
    void VulkanRenderTarget::createReadbackBuffers() {
        // 정수 어태치먼트가 없으면 읽을 게 없다 (셰이더 프리뷰 타겟 등)
        bool needsReadback = false;
        for (const ColorAttachment& c : m_Colors) {
            if (!c.sampled) { needsReadback = true; break; }
        }
        if (!needsReadback) return;

        VmaAllocator allocator = VulkanContext::Get()->getAllocator();

        VkBufferCreateInfo bufCI{};
        bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufCI.size = sizeof(int32_t);   // 텍셀 딱 하나
        bufCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocCI{};
        allocCI.usage = VMA_MEMORY_USAGE_AUTO;
        // CPU가 읽어야 하므로 host-visible. MAPPED로 상주 매핑을 받아둔다 —
        // 4바이트를 매 클릭마다 map/unmap할 이유가 없다.
        allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
            | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        m_Readbacks.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        for (Readback& rb : m_Readbacks) {
            VmaAllocationInfo info{};
            if (vmaCreateBuffer(allocator, &bufCI, &allocCI,
                &rb.buffer, &rb.alloc, &info) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("pixel readback buffer creation failed!");
                m_Readbacks.clear();
                return;
            }
            rb.mapped = info.pMappedData;
        }
    }

    void VulkanRenderTarget::destroyReadbackBuffers() {
        VmaAllocator allocator = VulkanContext::Get()->getAllocator();
        for (Readback& rb : m_Readbacks) {
            if (rb.buffer != VK_NULL_HANDLE) {
                vmaDestroyBuffer(allocator, rb.buffer, rb.alloc);
                rb.buffer = VK_NULL_HANDLE;
                rb.alloc = nullptr;
                rb.mapped = nullptr;
            }
        }
        m_Readbacks.clear();
    }

    void VulkanRenderTarget::requestPixelRead(int attachmentIndex, uint32_t x, uint32_t y) {
        if (m_Readbacks.empty()) return;
        if (attachmentIndex < 0 || attachmentIndex >= (int)m_Colors.size()) return;
        // 패널 크기와 타겟 크기는 리사이즈 도중 한 프레임 어긋날 수 있다.
        if (x >= m_Spec.w || y >= m_Spec.h) return;

        m_PendingReadAttachment = attachmentIndex;
        m_PendingReadX = x;
        m_PendingReadY = y;
    }

    bool VulkanRenderTarget::pollPixelRead(int32_t& outValue) {
        if (!m_HasResult) return false;
        outValue = m_Result;
        m_HasResult = false;   // 한 번 꺼내면 사라진다
        return true;
    }

    // endPass()가 매 프레임 부른다. 렌더패스 **밖**이어야 한다 — 전송 명령은
    // 렌더패스 안에서 기록할 수 없다.
    void VulkanRenderTarget::harvestAndRecordPixelRead(VkCommandBuffer cmd) {
        if (m_Readbacks.empty()) return;

        auto ctx = VulkanContext::Get();
        const uint32_t slot = ctx->getCurrentFrameIndex();
        Readback& rb = m_Readbacks[slot];

        // ── 1. 수확. 이 슬롯에 걸린 복사는 2프레임 전에 기록된 것이고,
        //    beginFrame이 이 슬롯을 재사용하기 전에 fence[slot]을 기다렸으므로
        //    지금 시점엔 반드시 끝나 있다. 그래서 여기서 기다릴 필요가 없다.
        if (rb.pending) {
            // 비코히런트 메모리면 CPU 캐시를 무효화해야 한다. 코히런트면 no-op.
            vmaInvalidateAllocation(ctx->getAllocator(), rb.alloc, 0, sizeof(int32_t));
            m_Result = *reinterpret_cast<const int32_t*>(rb.mapped);
            m_HasResult = true;
            rb.pending = false;
        }

        // ── 2. 새 요청 기록
        if (m_PendingReadAttachment < 0) return;

        const ColorAttachment& c = m_Colors[m_PendingReadAttachment];

        VkBufferImageCopy region{};
        // 0 = "이미지 영역에 딱 맞게 촘촘히" — 1x1이라 4바이트다
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { (int32_t)m_PendingReadX, (int32_t)m_PendingReadY, 0 };
        region.imageExtent = { 1, 1, 1 };

        // 레이아웃은 렌더패스 finalLayout이 이미 TRANSFER_SRC_OPTIMAL로 만들어놨다.
        // (sampled=false 슬롯의 finalLayout — createRenderPass 참고)
        vkCmdCopyImageToBuffer(cmd, c.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, rb.buffer, 1, &region);

        rb.pending = true;
        m_PendingReadAttachment = -1;   // 클릭 한 번에 복사 한 번
    }

    // =========================================================================
    // RenderPass — 어태치먼트 구성 기술서. 크기를 담지 않으므로 리사이즈와 무관.
    // =========================================================================
    void VulkanRenderTarget::createRenderPass() {
        VkDevice device = VulkanContext::Get()->getDevice();

        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference>   colorRefs;
        attachments.reserve(m_Colors.size() + 1);
        colorRefs.reserve(m_Colors.size());

        // attachment 0..N-1 — 색.
        // finalLayout이 스왑체인 패스와 갈리는 지점: 여기선 present가 아니다.
        //   sampled → ImGui가 셰이더로 읽는다   → SHADER_READ_ONLY_OPTIMAL
        //   그 외   → CPU가 복사해 간다(id 버퍼) → TRANSFER_SRC_OPTIMAL
        // ⚠️ P-b에서 실제로 복사를 넣을 땐 아래 deps[1]의 dst 마스크에
        //    TRANSFER/TRANSFER_READ를 더해야 한다.
        for (size_t i = 0; i < m_Colors.size(); ++i) {
            VkAttachmentDescription color{};
            color.format = m_Colors[i].format;
            color.samples = VK_SAMPLE_COUNT_1_BIT;
            color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;    // 나중에 읽으므로 보존
            color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color.finalLayout = m_Colors[i].sampled
                ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            attachments.push_back(color);

            VkAttachmentReference ref{};
            ref.attachment = (uint32_t)i;
            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs.push_back(ref);
        }

        // 마지막 attachment — 깊이. 다음 프레임에 다시 CLEAR하므로 저장할 필요 없음.
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

            // 깊이는 색 슬롯 뒤에 온다. 색이 늘면 이 번호도 같이 밀린다.
            depthRef.attachment = (uint32_t)m_Colors.size();
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = (uint32_t)colorRefs.size();
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = m_HasDepth ? &depthRef : nullptr;

        // 의존성 2개 — 이 FB는 매 프레임 "그리기 → 읽기"를 반복한다.
        //  [0] 이전 프레임 읽기가 끝난 뒤에 이번 프레임 쓰기를 시작
        //  [1] 이번 프레임 쓰기가 끝난 뒤에 읽기를 시작
        //
        // ⚠️ "읽기"가 두 종류다 — ImGui의 셰이더 샘플링(색)과 피킹의 전송 복사(id).
        //    P-b에서 vkCmdCopyImageToBuffer가 붙으면서 TRANSFER를 안 넣으면
        //    복사가 렌더링 완료보다 앞설 수 있다.
        //
        // ⚠️ BY_REGION은 뺐다. 그 플래그는 "같은 픽셀 위치끼리만 의존한다"는
        //    프레임버퍼 지역성 약속인데, 이미지 전체를 훑는 전송에는 성립하지 않는다.
        VkSubpassDependency deps[2]{};

        deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        deps[0].dstSubpass = 0;
        deps[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            | VK_PIPELINE_STAGE_TRANSFER_BIT;
        deps[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
        deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        deps[1].srcSubpass = 0;
        deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        deps[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            | VK_PIPELINE_STAGE_TRANSFER_BIT;
        deps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;

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

        // ── 색 이미지들: 전부 그리기 대상이고 그 뒤가 갈린다.
        //      sampled → SAMPLED      (ImGui가 셰이더로 읽는다)
        //      그 외   → TRANSFER_SRC (CPU가 복사해 간다 — id 버퍼)
        //    ⚠️ usage는 이미지 생성 시점에만 정할 수 있다.
        VkImageCreateInfo colorCI{};
        colorCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        colorCI.imageType = VK_IMAGE_TYPE_2D;
        colorCI.extent = { m_Spec.w, m_Spec.h, 1 };
        colorCI.mipLevels = 1;
        colorCI.arrayLayers = 1;
        colorCI.samples = VK_SAMPLE_COUNT_1_BIT;
        colorCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        colorCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        colorCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        for (ColorAttachment& c : m_Colors) {
            colorCI.format = c.format;
            colorCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                | (c.sampled ? VK_IMAGE_USAGE_SAMPLED_BIT
                             : VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

            if (vmaCreateImage(allocator, &colorCI, &allocCI,
                &c.image, &c.alloc, nullptr) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("offscreen color image creation failed!");
                return;
            }
            c.view = VulkanSwapChain::CreateImageView(device, c.image,
                c.format, VK_IMAGE_ASPECT_COLOR_BIT);
        }

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
        // ⚠️ createRenderPass가 쌓은 순서(색 전부 → 깊이)와 반드시 같아야 한다.
        std::vector<VkImageView> views;
        views.reserve(m_Colors.size() + 1);
        for (const ColorAttachment& c : m_Colors) views.push_back(c.view);
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
        //
        // ⚠️ sampled 슬롯만 대상이다. id 버퍼는 아무도 안 읽으므로 전환할 이유가 없고,
        //    렌더패스가 매 프레임 UNDEFINED에서 시작하니 그대로 둬도 안전하다.
        {
            std::vector<VkImageMemoryBarrier> barriers;
            for (const ColorAttachment& c : m_Colors) {
                if (!c.sampled) continue;
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = c.image;
                barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barriers.push_back(barrier);
            }

            if (!barriers.empty()) {
                VkCommandBuffer cmd = VulkanTransfer::BeginSingleTime(
                    device, ctx->getGraphicsCommandPool());

                vkCmdPipelineBarrier(cmd,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0, 0, nullptr, 0, nullptr,
                    (uint32_t)barriers.size(), barriers.data());

                VulkanTransfer::EndSingleTime(device, ctx->getGraphicsCommandPool(),
                    ctx->getGraphicsQueue(), cmd);
            }
        }

        // ── ImGui 디스크립터셋: 이 핸들이 곧 getColorTextureID()의 반환값.
        //    id 버퍼는 정수 포맷이라 선형 필터링이 안 되고 샘플링할 이유도 없다.
        for (ColorAttachment& c : m_Colors) {
            if (!c.sampled) continue;
            c.descriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, c.view,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        APP_WMS_CORE_INFO("offscreen RenderTarget {0}x{1} (color: {2}, depth: {3})",
            m_Spec.w, m_Spec.h, m_Colors.size(), m_HasDepth);
    }

    void VulkanRenderTarget::cleanup() {
        VkDevice     device = VulkanContext::Get()->getDevice();
        VmaAllocator allocator = VulkanContext::Get()->getAllocator();

        // 디스크립터셋을 반납하지 않으면 리사이즈마다 DescriptorPool이 샌다
        for (ColorAttachment& c : m_Colors) {
            if (c.descriptorSet != VK_NULL_HANDLE) {
                ImGui_ImplVulkan_RemoveTexture(c.descriptorSet);
                c.descriptorSet = VK_NULL_HANDLE;
            }
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
        // ⚠️ m_Colors 자체는 비우지 않는다 — 어떤 슬롯이 어떤 포맷인지는 spec에서
        //    온 구성이라 리사이즈로 바뀌지 않는다. 핸들만 비운다.
        for (ColorAttachment& c : m_Colors) {
            if (c.view != VK_NULL_HANDLE) {
                vkDestroyImageView(device, c.view, nullptr);
                c.view = VK_NULL_HANDLE;
            }
            if (c.image != VK_NULL_HANDLE) {
                vmaDestroyImage(allocator, c.image, c.alloc);
                c.image = VK_NULL_HANDLE;
                c.alloc = nullptr;
            }
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

        // clear 값 배열의 인덱스 = 어태치먼트 슬롯 번호. createRenderPass가 쌓은
        // 순서(색 전부 → 깊이)와 어긋나면 엉뚱한 슬롯이 지워진다.
        std::vector<VkClearValue> clears(m_Colors.size() + (m_HasDepth ? 1 : 0));
        for (size_t i = 0; i < m_Colors.size(); ++i) {
            if (m_Colors[i].sampled) {
                clears[i].color = { { cc.r, cc.g, cc.b, cc.a } };
            }
            else {
                // 🟢 id 버퍼의 "빈 곳" = -1. 비트패턴이 0xFFFFFFFF라 entt::null과
                //    정확히 같다(entt::null = construct(entity_mask, version_mask)).
                //    그래서 P-b에서 읽은 값을 그대로 entt::entity로 되돌리면
                //    빈 곳 클릭이 자연스럽게 "선택 없음"이 된다.
                // ⚠️ .float32가 아니라 .int32다. 정수 어태치먼트에 float를 쓰면
                //    비트가 그대로 들어가 -1이 아닌 쓰레기 값이 clear된다.
                clears[i].color.int32[0] = -1;
                clears[i].color.int32[1] = -1;
                clears[i].color.int32[2] = -1;
                clears[i].color.int32[3] = -1;
            }
        }
        if (m_HasDepth) clears.back().depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo rpBegin{};
        rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.renderPass = m_RenderPass;
        rpBegin.framebuffer = m_Framebuffer;
        rpBegin.renderArea.offset = { 0, 0 };
        rpBegin.renderArea.extent = { m_Spec.w, m_Spec.h };
        rpBegin.clearValueCount = (uint32_t)clears.size();
        rpBegin.pClearValues = clears.data();

        vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        // 뷰포트/시저는 동적 상태로 간다 — 파이프라인을 패널 크기마다
        // 다시 만들지 않기 위해서. 3단계에서 DYNAMIC_STATE로 선언한다.
        VkViewport viewport{ 0.0f, 0.0f, (float)m_Spec.w, (float)m_Spec.h, 0.0f, 1.0f };
        VkRect2D   scissor{ { 0, 0 }, { m_Spec.w, m_Spec.h } };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void VulkanRenderTarget::endPass() {
        VkCommandBuffer cmd = VulkanContext::Get()->getCurrentCommandBuffer();
        vkCmdEndRenderPass(cmd);
        // ⚠️ 반드시 EndRenderPass 뒤다 — 전송 명령은 렌더패스 안에서 못 쓴다.
        //    그리고 이 자리에선 id 이미지가 이미 TRANSFER_SRC_OPTIMAL이다.
        harvestAndRecordPixelRead(cmd);
    }

    void VulkanRenderTarget::resize(uint32_t w, uint32_t h) {
        if (w == 0 || h == 0) return;
        if (w == m_Spec.w && h == m_Spec.h) return;  // 매 프레임 호출되므로 같은 크기는 무시

        m_Spec.w = w;
        m_Spec.h = h;

        // ③ 결정 A안 — GPU가 옛 이미지를 다 쓸 때까지 기다린 뒤 파괴한다.
        // OpenGL은 드라이버가 지연 삭제를 대신 해줬지만 벌칸은 앱 책임.
        vkDeviceWaitIdle(VulkanContext::Get()->getDevice());

        // 진행 중이던 픽셀 읽기는 버린다. waitIdle 뒤라 복사 자체는 끝났지만,
        // 그 좌표는 **옛 크기 기준**이라 지금 화면과 대응이 안 된다.
        // (스테이징 버퍼 자체는 4B라 크기와 무관 → 재생성하지 않는다)
        for (Readback& rb : m_Readbacks) rb.pending = false;
        m_PendingReadAttachment = -1;
        m_HasResult = false;

        cleanup();
        invalidate();  // ② 운용 규칙: 렌더패스는 유지 (크기와 무관)
    }
}

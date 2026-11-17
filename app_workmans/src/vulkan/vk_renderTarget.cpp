#include "pch.h"
#include "vulkan/vk_renderTarget.h"
#include "vulkan/vulkanContext.h"
#include "vulkan/vk_swapChain.h"   // reuses CreateImageView / FindDepthFormat
#include "vulkan/vk_transfer.h"    // reuses BeginSingleTime / EndSingleTime
#include "vulkan/vulkanRenderer.h"
#include "renderer/rendererCommand.h"

#include "backends/imgui_impl_vulkan.h"

namespace AppWMs {

    VulkanRenderTarget::VulkanRenderTarget(const RenderTargetSpec& spec)
        : m_Spec(spec)
    {
        // Attachment interpretation -- N color + 0 or 1 depth.
        // The order listed is both the render pass slot number and the out location in the fragment shader.
        for (AttachmentFormat attach : m_Spec.attachments) {
            switch (attach) {
            case AttachmentFormat::RGBA8: {
                ColorAttachment c;
                c.format = VK_FORMAT_R8G8B8A8_UNORM;
                c.sampled = true;    // ImGui puts this in the viewport panel
                m_Colors.push_back(c);
                break;
            }
            case AttachmentFormat::R32SINT: {
                ColorAttachment c;
                // R32_SINT is a Vulkan mandatory format (COLOR_ATTACHMENT), so no availability query is needed.
                c.format = VK_FORMAT_R32_SINT;
                c.sampled = false;   // the reader is the CPU, not a shader (P-b readback)
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

        createRenderPass();       // size-independent -> exactly once, here
        createReadbackBuffers();  // 4B each, so this is size-independent too
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
    // Pixel readback -- staging buffers
    // =========================================================================
    void VulkanRenderTarget::createReadbackBuffers() {
        // With no integer attachment there is nothing to read (a shader preview target, say)
        bool needsReadback = false;
        for (const ColorAttachment& c : m_Colors) {
            if (!c.sampled) { needsReadback = true; break; }
        }
        if (!needsReadback) return;

        VmaAllocator allocator = VulkanContext::Get()->getAllocator();

        VkBufferCreateInfo bufCI{};
        bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufCI.size = sizeof(int32_t);   // exactly one texel
        bufCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocCI{};
        allocCI.usage = VMA_MEMORY_USAGE_AUTO;
        // The CPU has to read it, so host-visible. MAPPED gives us a persistent mapping --
        // there is no reason to map/unmap four bytes on every click.
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
        // The panel size and the target size can be one frame apart during a resize.
        if (x >= m_Spec.w || y >= m_Spec.h) return;

        m_PendingReadAttachment = attachmentIndex;
        m_PendingReadX = x;
        m_PendingReadY = y;
    }

    bool VulkanRenderTarget::pollPixelRead(int32_t& outValue) {
        if (!m_HasResult) return false;
        outValue = m_Result;
        m_HasResult = false;   // taking it once consumes it
        return true;
    }

    // endPass() calls this every frame. It must be *outside* the render pass --
    // transfer commands cannot be recorded inside one.
    void VulkanRenderTarget::harvestAndRecordPixelRead(VkCommandBuffer cmd) {
        if (m_Readbacks.empty()) return;

        auto ctx = VulkanContext::Get();
        const uint32_t slot = ctx->getCurrentFrameIndex();
        Readback& rb = m_Readbacks[slot];

        // -- 1. Harvest. The copy sitting in this slot was recorded two frames ago, and
        //    beginFrame waited on fence[slot] before reusing this slot, so by now it is
        //    guaranteed finished. That is why there is nothing to wait for here.
        if (rb.pending) {
            // Non-coherent memory needs the CPU cache invalidated. On coherent memory this is a no-op.
            vmaInvalidateAllocation(ctx->getAllocator(), rb.alloc, 0, sizeof(int32_t));
            m_Result = *reinterpret_cast<const int32_t*>(rb.mapped);
            m_HasResult = true;
            rb.pending = false;
        }

        // -- 2. Record the new request
        if (m_PendingReadAttachment < 0) return;

        const ColorAttachment& c = m_Colors[m_PendingReadAttachment];

        VkBufferImageCopy region{};
        // 0 = "tightly packed to the image region" -- at 1x1 that is four bytes
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { (int32_t)m_PendingReadX, (int32_t)m_PendingReadY, 0 };
        region.imageExtent = { 1, 1, 1 };

        // The layout is already TRANSFER_SRC_OPTIMAL, set by the render pass finalLayout.
        // (the finalLayout of a sampled=false slot -- see createRenderPass)
        vkCmdCopyImageToBuffer(cmd, c.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, rb.buffer, 1, &region);

        rb.pending = true;
        m_PendingReadAttachment = -1;   // one copy per click
    }

    // =========================================================================
    // RenderPass -- the attachment layout description. It holds no size, so resize does not touch it.
    // =========================================================================
    void VulkanRenderTarget::createRenderPass() {
        VkDevice device = VulkanContext::Get()->getDevice();

        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference>   colorRefs;
        attachments.reserve(m_Colors.size() + 1);
        colorRefs.reserve(m_Colors.size());

        // attachment 0..N-1 -- color.
        // This is where finalLayout diverges from the swapchain pass: here it is not present.
        //   sampled   -> ImGui reads it in a shader        -> SHADER_READ_ONLY_OPTIMAL
        //   otherwise -> the CPU copies it out (id buffer) -> TRANSFER_SRC_OPTIMAL
        // WARNING: when the actual copy goes in for P-b, deps[1] below needs TRANSFER /
        //    TRANSFER_READ added to its dst masks.
        for (size_t i = 0; i < m_Colors.size(); ++i) {
            VkAttachmentDescription color{};
            color.format = m_Colors[i].format;
            color.samples = VK_SAMPLE_COUNT_1_BIT;
            color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;    // kept, because it is read later
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

        // Last attachment -- depth. No need to store it; the next frame CLEARs it again.
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

            // Depth comes after the color slots. Add a color slot and this index shifts with it.
            depthRef.attachment = (uint32_t)m_Colors.size();
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = (uint32_t)colorRefs.size();
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = m_HasDepth ? &depthRef : nullptr;

        // Two dependencies -- this FB repeats "draw -> read" every frame.
        //  [0] start this frame's write only after the previous frame's read finished
        //  [1] start the read only after this frame's write finished
        //
        // WARNING: there are two kinds of "read" -- ImGui's shader sampling (color) and
        //    picking's transfer copy (id). Once vkCmdCopyImageToBuffer lands in P-b, leaving
        //    TRANSFER out would let the copy run ahead of rendering completion.
        //
        // WARNING: BY_REGION was left out. That flag is a framebuffer-locality promise -- "only the
        //    same pixel positions depend on each other" -- which does not hold for a transfer sweeping the whole image.
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
    // Size-bound resources -- destroyed and recreated on every resize
    // =========================================================================
    void VulkanRenderTarget::invalidate() {
        auto ctx = VulkanContext::Get();
        VkDevice     device = ctx->getDevice();
        VmaAllocator allocator = ctx->getAllocator();

        VmaAllocationCreateInfo allocCI{};
        allocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        // -- Color images: all of them are draw targets, and what follows diverges.
        //      sampled   -> SAMPLED      (ImGui reads it in a shader)
        //      otherwise -> TRANSFER_SRC (the CPU copies it out -- the id buffer)
        //    WARNING: usage can only be decided at image creation time.
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

        // -- Depth image: it owns its own at the viewport panel size (the reason 2-a split it off the swapchain)
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

        // -- Framebuffer: binds the views above to the render pass attachment slots (array order = slot number)
        // WARNING: must match the order createRenderPass built (all color, then depth).
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

        // -- Sampler: used when ImGui reads this color image as a texture
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

        // -- Initial layout transition (UNDEFINED -> SHADER_READ_ONLY_OPTIMAL)
        // The render pass uses initialLayout=UNDEFINED, so drawing is fine, but a single frame
        // that gets sampled without a beginPass() would raise a validation error.
        // Doing the transition once at creation closes that hole.
        //
        // WARNING: sampled slots only. Nobody reads the id buffer, so there is no reason to
        //    transition it, and the render pass starts from UNDEFINED every frame anyway.
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

        // -- ImGui descriptor set: this handle is exactly what getColorTextureID() returns.
        //    The id buffer is an integer format, so it cannot be linearly filtered and there is no reason to sample it.
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

        // Failing to return the descriptor sets leaks the DescriptorPool on every resize
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
        // WARNING: m_Colors itself is not cleared -- which slot has which format is configuration
        //    that came from the spec and does not change on resize. Only the handles are cleared.
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

        // The clear color uses the value RendererCommand::setClearColor stored.
        // On Vulkan clear() is a no-op; loadOp=CLEAR does the actual clearing.
        const glm::vec4& cc =
            static_cast<VulkanRendererAPI*>(&RendererCommand::getAPI())->getClearColor();

        // Index into the clear value array = attachment slot number. Diverge from the order
        // createRenderPass built (all color, then depth) and the wrong slot gets cleared.
        std::vector<VkClearValue> clears(m_Colors.size() + (m_HasDepth ? 1 : 0));
        for (size_t i = 0; i < m_Colors.size(); ++i) {
            if (m_Colors[i].sampled) {
                clears[i].color = { { cc.r, cc.g, cc.b, cc.a } };
            }
            else {
                // "Empty" in the id buffer = -1. Its bit pattern is 0xFFFFFFFF, which is
                //    exactly entt::null (entt::null = construct(entity_mask, version_mask)).
                //    So casting the value read in P-b straight back to entt::entity makes
                //    a click on empty space fall out naturally as "no selection".
                // WARNING: .int32, not .float32. Using float on an integer attachment puts
                //    the bits in verbatim, clearing to garbage instead of -1.
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

        // Viewport/scissor go in as dynamic state, so the pipeline is not rebuilt for
        // every panel size. Step 3 declares them as DYNAMIC_STATE.
        VkViewport viewport{ 0.0f, 0.0f, (float)m_Spec.w, (float)m_Spec.h, 0.0f, 1.0f };
        VkRect2D   scissor{ { 0, 0 }, { m_Spec.w, m_Spec.h } };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void VulkanRenderTarget::endPass() {
        VkCommandBuffer cmd = VulkanContext::Get()->getCurrentCommandBuffer();
        vkCmdEndRenderPass(cmd);
        // WARNING: this must come after EndRenderPass -- transfer commands cannot be used inside a render pass.
        //    And at this point the id image is already in TRANSFER_SRC_OPTIMAL.
        harvestAndRecordPixelRead(cmd);
    }

    void VulkanRenderTarget::resize(uint32_t w, uint32_t h) {
        if (w == 0 || h == 0) return;
        if (w == m_Spec.w && h == m_Spec.h) return;  // called every frame, so ignore an unchanged size

        m_Spec.w = w;
        m_Spec.h = h;

        // Decision #3, option A -- wait until the GPU is done with the old images, then destroy.
        // On OpenGL the driver deferred the delete for us; on Vulkan it is the app's job.
        vkDeviceWaitIdle(VulkanContext::Get()->getDevice());

        // Any pixel read in flight is dropped. After waitIdle the copy itself is done, but
        // its coordinates are in terms of the *old* size and no longer map to what is on screen.
        // (the staging buffer itself is 4B, size-independent -> not recreated)
        for (Readback& rb : m_Readbacks) rb.pending = false;
        m_PendingReadAttachment = -1;
        m_HasResult = false;

        cleanup();
        invalidate();  // decision #2 operating rule: keep the render pass (it is size-independent)
    }
}

#include "pch.h"
#include "vulkanRenderer.h"
#include "vulkan/vulkanContext.h" // for access to device info
#include "vulkan/vk_vertexArray.h" // bindToCommandBuffer
#include "vulkan/vk_pipeline.h"    // pushConstants

namespace AppWMs {

    void VulkanRendererAPI::init() {
        APP_WMS_CORE_INFO("VulkanRendererAPI: RenderPass & DescriptorPool !");

        VkDevice device = VulkanContext::Get()->getDevice();
        VkFormat swapChainImageFormat = VulkanContext::Get()->getSwapChainImageFormat();

        // =========================================================================
        // 1. Assemble the RenderPass (the canvas setup)
        // =========================================================================
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear the screen before drawing
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // keep what was drawn
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // layout for final presentation

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // WARNING: the swapchain pass is ImGui only -> no depth attachment.
        //    The offscreen RenderPass is what handles 3D depth testing.

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
            APP_WMS_CORE_ERROR("Failed to create RenderPass!");
        }

        // =========================================================================
        // 2. Assemble the DescriptorPool
        // =========================================================================
        // Sized generously so ImGui and the engine textures have plenty of room.
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
        pool_info.maxSets = 1000 * 11; // generous maximum set count
        pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_info.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(device, &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("Failed to create DescriptorPool!");
        }
        recreateFramebuffers();
        APP_WMS_CORE_INFO("VulkanRendererAPI: RenderPass & DescriptorPool created!");
    }
    void VulkanRendererAPI::recreateFramebuffers() {
        auto ctx = VulkanContext::Get();
        VkDevice device = ctx->getDevice();

        // Destroy the existing ones (for the resize re-entry -- empty on the first call, so a no-op)
        for (auto fb : m_vkFramebuffers) {
            if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(device, fb, nullptr);
        }
        m_vkFramebuffers.clear();

        const auto& colorViews = ctx->getSwapChainImageViews();
        VkExtent2D  extent = ctx->getSwapChainExtent();

        m_vkFramebuffers.resize(colorViews.size());

        for (size_t i = 0; i < colorViews.size(); i++) {
            // Array order = attachment index in the RenderPass (0=color)
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
                APP_WMS_CORE_ERROR("Failed to create Framebuffer! (index {0})", i);
            }
        }

        APP_WMS_CORE_INFO("Created {0} framebuffers ({1}x{2})",
            m_vkFramebuffers.size(), extent.width, extent.height);
    }

    bool VulkanRendererAPI::beginFrame() {
        auto ctx = VulkanContext::Get();

        // Swapchain recreation happens only here. It is safe because the command buffer is not open yet.
        //    Why Context does not do it itself: the framebuffers are owned by the layer above (this class),
        //    so this is the side that knows the recreation order, and the rule that a lower layer never back-references a higher one still holds.
        if (ctx->isSwapChainDirty()) {
            if (!ctx->recreateSwapChain()) {
                // Cannot build it yet (minimized, etc.) -> keep the stale mark and skip this frame
                return false;
            }
            // The swapchain image views changed wholesale, so the FBs holding them are rebuilt too
            recreateFramebuffers();
        }

        // Context does the acquire + begins the command buffer. Out-of-date gives false -> skip the frame
        return ctx->beginFrame();
    }

    void VulkanRendererAPI::endFrame() {
        // Context ends the command buffer + submit + present + rotates the slot
        VulkanContext::Get()->swapBuffers();
    }


    // ─────────────────────────────────────────────────────────────
    // Stubs -- present to fill out the vtable. Filled in at the relevant step.
    // ─────────────────────────────────────────────────────────────
    void VulkanRendererAPI::set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
        // A window resize makes the swapchain stale. We only mark it here;
        // the actual recreation is handled by the next beginFrame (a frame boundary).
        // WARNING: this event can arrive mid-frame, while the command buffer is being recorded.
        VulkanContext::Get()->requestSwapChainRecreate();
    }

    void VulkanRendererAPI::setClearColor(const glm::vec4& color) {
        m_ClearColor = color;
    }

    void VulkanRendererAPI::clear() {
        // On Vulkan the RenderPass loadOp=CLEAR handles this -> no-op
    }

    void VulkanRendererAPI::drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {
        if (!vertexArray) return;

        // WARNING: precondition -- the caller must have opened the render pass (RenderTarget::beginPass)
        //    and bound a pipeline. Vulkan has no global state, so this function cannot check that itself.
        auto vkVA = std::static_pointer_cast<VulkanVertexArray>(vertexArray);

        const Ref<IndexBuffer>& ib = vertexArray->getIndexBuffer();
        if (!ib) {
            APP_WMS_CORE_ERROR("drawIndexed: no IndexBuffer");
            return;
        }

        VkCommandBuffer cmd = VulkanContext::Get()->getCurrentCommandBuffer();

        // Bind the vertex/index buffers into the command buffer (handles were cached in addVertexBuffer)
        vkVA->bindToCommandBuffer(cmd);

        // indexCount 0 = "draw everything" (the default convention of the existing abstract signature)
        const uint32_t count = indexCount ? indexCount : ib->getCount();

        vkCmdDrawIndexed(cmd, count, 1, 0, 0, 0);
    }

    void VulkanRendererAPI::drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
        // later
    }

    void VulkanRendererAPI::setLineWidth(float width) {
        // later (VK_DYNAMIC_STATE_LINE_WIDTH)
    }

    void VulkanRendererAPI::pushObjectMatrices(const glm::mat4& mvp, const glm::mat4& model,
        int32_t entityID) {
        if (!m_CurrentPipeline) {
            APP_WMS_CORE_ERROR("pushObjectMatrices: no pipeline is bound");
            return;
        }
        // WARNING: must match the Push block in cube.vert/frag byte for byte.
        //    A mismatch compiles and passes validation, and only the image breaks, silently.
        struct Push {
            glm::mat4 mvp;        //   0..63
            glm::mat4 model;      //  64..127
            int32_t   entityID;   // 128..131
        } data{ mvp, model, entityID };
        static_assert(sizeof(Push) == 132, "size diverged from the Push block in cube.vert/frag");

        m_CurrentPipeline->pushConstants(
            VulkanContext::Get()->getCurrentCommandBuffer(), &data, sizeof(data));
    }

    void VulkanRendererAPI::waitIdle() {
        vkDeviceWaitIdle(VulkanContext::Get()->getDevice());
    }

    void VulkanRendererAPI::shutdown() {
        // Precondition: App calls this only after waitIdle -> layer destruction -> ImGui backend shutdown.
        //       The descriptor pool was the one ImGui used, so it must not be destroyed before that.
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
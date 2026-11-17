#pragma once
#include "renderer/renderTarget.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace AppWMs {

    // Offscreen render target. It draws into its own images rather than the swapchain,
    // and ImGui samples the result as a texture to put it in the viewport panel.
    //
    // Decision #2, option A -- this class owns the VkRenderPass as a member (one per target).
    //   Why: a render pass bakes in the attachment layout, so sharing one would force
    //        every target to the same layout. With picking, the viewport target
    //        (RGBA8+R32_SINT+Depth) and the preview target (RGBA8) diverge, so sharing fails.
    class VulkanRenderTarget : public RenderTarget {
    public:
        VulkanRenderTarget(const RenderTargetSpec& spec);
        virtual ~VulkanRenderTarget();

        virtual void beginPass() override;  // vkCmdBeginRenderPass
        virtual void endPass() override;    // vkCmdEndRenderPass
        virtual void resize(uint32_t w, uint32_t h) override;

        virtual const RenderTargetSpec& get_spec() const override { return m_Spec; }

        // The handle passed straight to ImGui::Image(). On Vulkan that is a VkDescriptorSet.
        // index is the color attachment number (depth is not counted). A slot that is not
        // a sampling target (R32_SINT) has no descriptor set, so it returns 0.
        virtual uint64_t getColorTextureID(int index = 0) const override {
            if (index < 0 || index >= (int)m_Colors.size()) return 0;
            return (uint64_t)m_Colors[index].descriptorSet;
        }

        // Where the pipeline (step 3) borrows the target render pass
        VkRenderPass getRenderPass() const { return m_RenderPass; }

        // WARNING: the pipeline's color blend attachment count must equal the subpass'
        //    colorAttachmentCount. A mismatch makes pipeline creation fail.
        uint32_t getColorAttachmentCount() const { return (uint32_t)m_Colors.size(); }

        virtual void requestPixelRead(int attachmentIndex, uint32_t x, uint32_t y) override;
        virtual bool pollPixelRead(int32_t& outValue) override;

    private:
        void createRenderPass();  // once in the constructor. Size-independent, so not recreated on resize
        void invalidate();        // creates images/views/framebuffer/sampler/descriptor sets
        void cleanup();           // destroys only the above (render pass excluded)

        // Pixel readback -- the staging buffer is 4 bytes, so it is size-independent.
        // That is why the constructor/destructor own it, not invalidate/cleanup.
        void createReadbackBuffers();
        void destroyReadbackBuffers();
        // endPass() calls this every frame: harvest what is ready -> record the new request
        void harvestAndRecordPixelRead(VkCommandBuffer cmd);

    private:
        RenderTargetSpec m_Spec;

        // N color attachments. The order listed is both the render pass slot number
        // and the out location in the fragment shader.
        //   slot 0: RGBA8    -- drawing + ImGui sampling
        //   slot 1: R32_SINT -- drawing + CPU readback (P-b). Never sampled
        struct ColorAttachment {
            VkImage         image = VK_NULL_HANDLE;
            VmaAllocation   alloc = nullptr;
            VkImageView     view = VK_NULL_HANDLE;
            VkFormat        format = VK_FORMAT_UNDEFINED;
            // With sampled=false we create no SAMPLED usage, no descriptor set and no initial
            // layout transition. An integer format cannot be linearly filtered at all.
            bool            sampled = false;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        };
        std::vector<ColorAttachment> m_Colors;

        // Depth attachment -- only when spec.attachments contains Depth
        VkImage       m_DepthImage = VK_NULL_HANDLE;
        VmaAllocation m_DepthAlloc = nullptr;
        VkImageView   m_DepthView = VK_NULL_HANDLE;
        VkFormat      m_DepthFormat = VK_FORMAT_UNDEFINED;
        bool          m_HasDepth = false;

        // Decision #2, option A -- one owned per target
        VkRenderPass  m_RenderPass = VK_NULL_HANDLE;
        // WARNING: this is the Vulkan API handle (the slot wiring table), not the same concept as this class.
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

        // The ImGui sampling path. The sampler is shared by the sampled slots.
        VkSampler       m_Sampler = VK_NULL_HANDLE;

        // -- Pixel readback (P-b) ---------------------------------------------
        // WARNING: the key is having one per frame-in-flight slot. If the copy is recorded
        //    into slot S, then two frames later, when we come back to that slot, beginFrame
        //    has already waited on fence[S], so copy completion is guaranteed for free.
        //    Recycling a single buffer loses that guarantee and forces a vkDeviceWaitIdle.
        struct Readback {
            VkBuffer      buffer = VK_NULL_HANDLE;
            VmaAllocation alloc = nullptr;
            void* mapped = nullptr;   // persistently mapped (at 4B there is no reason to map/unmap each time)
            bool          pending = false;     // is there a copy in this slot waiting to be harvested
        };
        std::vector<Readback> m_Readbacks;   // [frame-in-flight slot]

        // The request to record in the next endPass(). -1 means no request.
        int      m_PendingReadAttachment = -1;
        uint32_t m_PendingReadX = 0, m_PendingReadY = 0;

        // The harvested result. Goes back to false once pollPixelRead takes it.
        bool     m_HasResult = false;
        int32_t  m_Result = -1;
    };
}

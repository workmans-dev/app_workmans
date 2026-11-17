#pragma once

#include "core/core.h"
#include <vector>

namespace AppWMs {

    // What a single image attached to a render target is for.
    enum class AttachmentFormat {
        None = 0,
        RGBA8,          // color
        // P-a -- the entity id buffer for picking. It is data, not color, so ImGui does not
        //    sample it. Empty areas clear to -1, whose bit pattern equals entt::null.
        R32SINT,
        DEPTHSTENCIL8,  // depth (+ stencil)
        Depth = DEPTHSTENCIL8
    };

    struct RenderTargetSpec {
        uint32_t w = 0, h = 0;
        uint32_t samples = 1;
        // Listed like { RGBA8, Depth }. The order listed is the attachment slot number.
        std::vector<AttachmentFormat> attachments;
    };

    // "Where do we draw" -- a target that draws into its own images rather than the screen.
    //
    // WARNING: the old name was FrameBuffer. That was OpenGL FBO terminology and did not match
    //    the role: VkFramebuffer is just one part this class holds, and it holds no pixels.
    //    What this class actually is: a render target that owns, together, the images drawn into
    //    and the path for reading the result back as a texture. The concept D3D/Unity/UE call a render target.
    class RenderTarget {
    public:
        static Ref<RenderTarget> create(const RenderTargetSpec& spec);

        virtual ~RenderTarget() = default;

        // WARNING: not bind/unbind. Vulkan has no global state called "the currently bound target".
        //    What this actually does is open and close a render pass on the command buffer, and
        //    the commands pile up in the command buffer, not in this object.
        //    The implementation borrows the command buffer from the context, so there is no argument.
        virtual void beginPass() = 0;
        virtual void endPass() = 0;

        virtual void resize(uint32_t w, uint32_t h) = 0;

        virtual const RenderTargetSpec& get_spec() const = 0;

        // The texture handle passed straight to ImGui::Image().
        // On Vulkan that is a VkDescriptorSet (64-bit) -> matched in width to ImTextureID (ImU64).
        virtual uint64_t getColorTextureID(int index = 0) const = 0;

        // P-b -- reads a single pixel from an integer attachment (R32_SINT) back to the CPU.
        //
        // WARNING: *the result is not immediate.* Splitting the request from the harvest is not a
        //    preference but a constraint: reading a pixel just drawn would mean waiting for the GPU
        //    to finish that frame, which drains the pipeline and stalls the frame.
        //    So only the copy is scheduled, and it is picked up a few frames later.
        //
        // (x, y) are texel coordinates from the attachment's top-left. Out-of-range is ignored.
        virtual void requestPixelRead(int attachmentIndex, uint32_t x, uint32_t y) = 0;

        // If a value is ready it is taken out and true returned. Otherwise nothing is touched and false.
        // Taking it once consumes it -- calling every frame does not repeat the same value.
        virtual bool pollPixelRead(int32_t& outValue) = 0;
    };
}

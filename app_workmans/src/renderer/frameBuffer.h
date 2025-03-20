#pragma once

#include "core/core.h"
#include <vector>

namespace AppWMs {
	enum class FramebufferTextureFormat {
		None = 0, RGBA8, DEPTHSTENCIL8, Depth = DEPTHSTENCIL8
	};

	struct FrameBufferTextureSpec {
		FrameBufferTextureSpec() = default;
		FrameBufferTextureSpec(FramebufferTextureFormat format) :textFormat(format) {}
		FramebufferTextureFormat textFormat;
	};
	struct FrameBufferAttachmentSpec {
		FrameBufferAttachmentSpec() = default ;
		FrameBufferAttachmentSpec(std::initializer_list<FrameBufferTextureSpec> attachments)
			:attachments(attachments) {}

		std::vector<FrameBufferTextureSpec> attachments;

	};

	struct FrameBufferSpec {
		uint32_t w, h;
		uint32_t samples=1;
		FrameBufferAttachmentSpec attachSpec;
		bool swapChainTarget = false;
	};

	class FrameBuffer {
	public:

		static Ref<FrameBuffer> create(const FrameBufferSpec& spec);

		virtual ~FrameBuffer() = default;
		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual void resize(uint32_t w, uint32_t h) =0;

	public:
		virtual const FrameBufferSpec& get_spec() const = 0;
		virtual uint32_t get_colorAttach() const = 0;
		virtual uint32_t get_renderID() const = 0;
	};
}
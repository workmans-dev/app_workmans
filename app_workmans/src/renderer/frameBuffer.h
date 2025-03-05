#pragma once

#include "core/core.h"


namespace AppWMs {

	struct FrameBufferSpec {
		uint32_t w, h;
		uint32_t samples=1;
		bool swapChainTarget = false;
	};

	class FrameBuffer {
	public:
		virtual ~FrameBuffer() = default;
		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual void resize(uint32_t w, uint32_t h) =0;

		static Ref<FrameBuffer> create(const FrameBufferSpec& spec);

	public:
		virtual const FrameBufferSpec& get_spec() const = 0;
		virtual uint32_t get_colorAttach() const = 0;
		virtual uint32_t get_renderID() const = 0;
	};
}
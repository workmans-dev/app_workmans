#pragma once
#include "renderer/frameBuffer.h"
namespace AppWMs {
	class OpenGLframeBuffer : public FrameBuffer{
	public:
		OpenGLframeBuffer(const FrameBufferSpec& spec);
		virtual ~OpenGLframeBuffer();
		void invalidate();

		virtual void bind() override;
		virtual void unbind() override;

		virtual void resize(uint32_t w, uint32_t h) override;
		
	public:
		virtual const FrameBufferSpec& get_spec() const override{ return pm_spec; }
		virtual uint32_t get_renderID() const override { return pm_renderID; }
		virtual uint32_t get_colorAttach() const override { return pm_colorAttach; }

	private:
		uint32_t pm_renderID=0;
		uint32_t pm_colorAttach = 0, pm_depthAttach = 0;
		FrameBufferSpec pm_spec;
	};

}
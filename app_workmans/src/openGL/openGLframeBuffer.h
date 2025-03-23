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
		virtual const FrameBufferSpec& get_spec() const override{ return pm_bufferSpec; }
		virtual uint32_t get_renderID() const override { return pm_renderID; }
		virtual uint32_t get_colorAttach(int index=0) const override { return pm_textureID_vec[index]; }

	private:
		uint32_t pm_renderID=0;
		//uint32_t pm_colorAttach = 0, pm_depthAttach = 0;
		FrameBufferSpec pm_bufferSpec;

		std::vector<FrameBufferTextureSpec> pm_textureSpec_vec;
		FrameBufferTextureSpec pm_depthSpec;

		std::vector<uint32_t> pm_textureID_vec;
		uint32_t pm_depthID;
	};

}
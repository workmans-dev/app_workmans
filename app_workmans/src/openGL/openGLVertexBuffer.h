#pragma once
#include "renderer/buffer.h"

namespace AppWMs {
	
	// vertexBuffer
	class OpenGLVertexBuffer :public VertexBuffer {
	public:
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		OpenGLVertexBuffer(uint32_t size);
		virtual ~OpenGLVertexBuffer();
		virtual void bind() const override;
		virtual void unbind() const override;
		virtual const BufferLayout& getLayout() const override { return pm_layout; }
		virtual void setLayout(const BufferLayout& layout) override { pm_layout = layout; }
		virtual void setData(const void* VB, uint32_t size) override;
	private:
		uint32_t pm_renderID;
		BufferLayout pm_layout;
	};


	// indexBuffer
	class OpenGLIndexBuffer :public IndexBuffer {
	public:
		OpenGLIndexBuffer(uint32_t* indexes, uint32_t count);
		virtual ~OpenGLIndexBuffer();
		virtual void bind() const override;
		virtual void unbind() const override;
		virtual uint32_t getCount() const { return pm_count; }
	private:
		uint32_t pm_renderID;
		uint32_t pm_count;
	};

}
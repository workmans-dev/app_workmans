#pragma once
#include <vector>
#include "renderer/vertexArray.h"

namespace AppWMs{
	class OpenGLVertexArray : public VertexArray {
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();
		virtual void bind() const override;
		virtual void unbind() const override;

		virtual void addVertexBuffer(const Ref<VertexBuffer> vertexBuffer) override;
		virtual void setIndexBuffer(const Ref<IndexBuffer> indexBuffer) override;

		virtual const std::vector<Ref<VertexBuffer>> getVertexBuffers() const { return pm_vertexBuffers; }
		virtual const Ref<IndexBuffer> getIndexBuffer() const { return pm_indexBuffer; }
	private:
		std::vector<Ref<VertexBuffer>> pm_vertexBuffers;
		Ref<IndexBuffer> pm_indexBuffer;
		uint32_t pm_renderID;
	};



}
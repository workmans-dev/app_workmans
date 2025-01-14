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

		virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer> vertexBuffer) override;
		virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer> indexBuffer) override;

		virtual const std::vector<std::shared_ptr<VertexBuffer>> getVertexBuffers() const { return pm_vertexBuffers; }
		virtual const std::shared_ptr<IndexBuffer> getIndexBuffer() const { return pm_indexBuffer; }
	private:
		std::vector<std::shared_ptr<VertexBuffer>> pm_vertexBuffers;
		std::shared_ptr<IndexBuffer> pm_indexBuffer;
		uint32_t pm_renderID;
	};



}
#pragma once
#include "core/core.h"
#include "renderer/rendererAPI.h"

namespace AppWMs {
	class OpenGLRendererAPI : public RendererAPI{
	public:
		virtual void init() override;
		virtual void set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
		virtual void setClearColor(const glm::vec4& color) override;
		virtual void clear() override;
		virtual void drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount=0) override;
		virtual void drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void setLineWidth(float width) override;
	};
}
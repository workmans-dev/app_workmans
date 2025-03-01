#pragma once
#include "renderer/rendererAPI.h"

namespace AppWMs {
	class OpenGLRendererAPI : public RendererAPI{
	public:
		virtual void init() override;
		virtual void set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
		virtual void setClearColor(const glm::vec4& color) override;
		virtual void clear() override;
		virtual void drawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
}
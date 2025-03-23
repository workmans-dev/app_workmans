#pragma once

#include <glm/glm.hpp>
#include "renderer/vertexArray.h"

namespace AppWMs {
	class RendererAPI {
	public:
		enum class SelectRenderer {
			None = 0, OpenGL = 1
		};
	public:
		virtual void init() = 0;
		virtual void set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) = 0;
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual void clear() =0;
		virtual void drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount=0) = 0;
		virtual void drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void setLineWidth(float width) = 0;
		inline static RendererAPI::SelectRenderer getRenderer() { return s_render; }
	private:
		static SelectRenderer s_render;
	};

}
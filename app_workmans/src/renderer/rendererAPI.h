#pragma once

#include <glm/glm.hpp>
#include "renderer/vertexArray.h"

namespace AppWMs {
	class RendererAPI {
	public:
		enum class SelectRenderer {
			None = 0, OpenGL = 1
		};

		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual void clear() =0;
		virtual void drawIndexed(const Ref<VertexArray>& vertexArray) = 0;

		inline static RendererAPI::SelectRenderer getRenderer() { return s_render; }
	private:
		static SelectRenderer s_render;
	};

}
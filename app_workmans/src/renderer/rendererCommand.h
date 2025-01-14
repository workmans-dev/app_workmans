#pragma once

#include "renderer/rendererAPI.h"

namespace AppWMs {
	class RendererCommand {
	public:
		inline static void setClearColor(const glm::vec4& color) { 
			pm_rendererAPI_ptr->setClearColor(color); 
		}
		inline static void clear() { 
			pm_rendererAPI_ptr->clear(); 
		}
		inline static void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
			pm_rendererAPI_ptr->drawIndexed(vertexArray);
		}
	private:
		static RendererAPI* pm_rendererAPI_ptr;

	};

}
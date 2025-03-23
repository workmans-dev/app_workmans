#pragma once

#include "renderer/rendererAPI.h"

namespace AppWMs {
	class RendererCommand {
	public:
		inline static void init() { pm_rendererAPI_ptr->init(); }
		inline static void set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
		{
			pm_rendererAPI_ptr->set_viewport(x, y, w, h);
		}
		inline static void setClearColor(const glm::vec4& color) { 
			pm_rendererAPI_ptr->setClearColor(color); 
		}
		inline static void clear() { 
			pm_rendererAPI_ptr->clear(); 
		}
		inline static void drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount=0) {
			pm_rendererAPI_ptr->drawIndexed(vertexArray, indexCount);
		}
	private:
		//  in cpp file, it will create as OpenGLRenderAPI pointer.
		static Scope<RendererAPI> pm_rendererAPI_ptr;

	};

}
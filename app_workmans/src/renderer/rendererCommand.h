#pragma once

#include "renderer/rendererAPI.h"

namespace AppWMs {
	class RendererCommand {
	public:
		inline static void init() { pm_rendererAPI_ptr->init(); }
		inline static bool beginFrame() { return pm_rendererAPI_ptr->beginFrame(); }
		inline static void endFrame() { pm_rendererAPI_ptr->endFrame(); }

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
		inline static void pushObjectMatrices(const glm::mat4& mvp, const glm::mat4& model) {
			pm_rendererAPI_ptr->pushObjectMatrices(mvp, model);
		}
		inline static void waitIdle() { pm_rendererAPI_ptr->waitIdle(); }
		inline static void shutdown() { pm_rendererAPI_ptr->shutdown(); }
		inline static RendererAPI& getAPI() { return *pm_rendererAPI_ptr; }
	private:
		static Scope<RendererAPI> pm_rendererAPI_ptr;

	};

}
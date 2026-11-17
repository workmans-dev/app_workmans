#pragma once

#include <glm/glm.hpp>
#include "renderer/vertexArray.h"

namespace AppWMs {
	class RendererAPI {
	public:
		enum class SelectRenderer {
			None = 0, Vulkan = 1
		};
	public:
		virtual void init() = 0;
		// Frame lifecycle (the backend hides acquire/submit/present)
		virtual bool beginFrame() = 0;
		virtual void endFrame() = 0;
		virtual void set_viewport(uint32_t x, uint32_t y,
			uint32_t w, uint32_t h) = 0;
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual void clear() =0;
		virtual void drawIndexed(const Ref<VertexArray>& vertexArray, 
			uint32_t indexCount=0) = 0;
		virtual void drawLines(const Ref<VertexArray>& vertexArray, 
			uint32_t vertexCount) = 0;
		virtual void setLineWidth(float width) = 0;
		// Passes the per-object transform matrices.
		//    The Vulkan implementation sends them as push constants of the currently bound pipeline.
		//    Once materials (UBO/descriptor sets) exist, this spot extends that way.
		//    entityID is the value stamped into the picking id buffer. -1 means "none", and its bit
		//    pattern equals entt::null, so a read value can be cast straight back to an entity.
		//    WARNING: not using entt types here is deliberate -- the renderer does not know the scene.
		virtual void pushObjectMatrices(const glm::mat4& mvp, const glm::mat4& model,
			int32_t entityID) = 0;
		// Waits until the GPU has finished all submitted work.
		//    Called once before destroying resources at shutdown, to avoid destroying in-use handles.
		virtual void waitIdle() = 0;
		// Releases the resources the backend owns. Why this is not left to the destructor:
		//    the implementation has static lifetime (RendererCommand's unique_ptr), so it would be
		//    destroyed later than the graphics device. App controls the shutdown order.
		virtual void shutdown() = 0;
		inline static RendererAPI::SelectRenderer getRenderer() { return s_render; }
	private:
		static SelectRenderer s_render;
	};

}
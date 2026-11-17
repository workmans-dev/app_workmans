#pragma once
#include "renderer/rendererCommand.h"
#include "renderer/camera/cameraController.h"
//#include "renderer/shader.h"

namespace AppWMs {

	class Renderer {
	public:
		static void init();
		static void shutdown();
		static bool beginFrame() { return RendererCommand::beginFrame(); }
		static void endFrame() { RendererCommand::endFrame(); }
		static void onWindowResize(uint32_t w, uint32_t h);
		// start render
		// viewProj must already have the Vulkan correction (proj[1][1] *= -1) applied by the caller.
		static void beginScene(const glm::mat4& viewProj);
		// Decision #4, option C -- the pipeline is whatever the caller bound beforehand.
		// Once a per-pipeline sorting loop exists, that loop manages the binding.
		// entityID is the value stamped into the picking id buffer. The default -1 means "none"
		// (= entt::null's bit pattern), so clicking a pixel drawn with it picks up nothing.
		static void submit(const Ref<VertexArray>& vertexArray,
			const glm::mat4& model = glm::mat4(1.0f),
			int32_t entityID = -1);
		static void end();

		inline static RendererAPI::SelectRenderer getRenderer() { return RendererAPI::getRenderer(); }

	private:
		// How this frame's camera sees the world = Projection * View.
		//
		// WARNING: Model (the world matrix) is not here. It differs per object, so submit takes it as an argument.
		//    MVP = pm_cameraViewProj * model is completed inside submit.
		//
		// There is one camera per frame but many objects, so this is composed once here and
		// each object only multiplies in its model. beginScene fills it; submit reads it.
		//
		// Once per-frame shared values such as lights and camera position pile up, they get a struct.
		// For now there is only one field, so the heap allocation (Scope) was removed.
		static glm::mat4 pm_cameraViewProj;
	};
}
#pragma once
#include "renderer/rendererCommand.h"
#include "renderer/camera/camera.h"
#include "renderer/shader.h"

namespace AppWMs {

	class Renderer {
	public:
		static void init();
		static void onWindowResize(uint32_t w, uint32_t h);
		// start render
		static void beginScene(const CameraMatrix& camera);
		static void submit(const Ref<VertexArray>& vertexArray, const Ref<Shader> shader,
			const glm::mat4& transform=glm::mat4(1.0f));
		static void end();

		inline static RendererAPI::SelectRenderer getRenderer() { return RendererAPI::getRenderer(); }

	private:
		struct SceneData {
			glm::mat4 view_proj_mat;
		};
		static Scope<SceneData> pm_sceneData;
	};
}
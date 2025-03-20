#pragma once

#include "renderer/camera/cameraController.h"
#include "core/layer.h"
#include "renderer/vertexArray.h"
#include "core/core.h"
#include "renderer/texture.h"
#include "renderer/shader.h"
#include <glm/gtc/matrix_transform.hpp>

namespace AppWMs {
	class Renderer2D {
	public:
		static void init();
		static void beginScene(const CameraController& cam, const glm::mat4& transform);

		static void endScene();

		static void drawQuad(const glm::mat4& transform, const glm::vec4& color );
		static void drawQuad_texture(const std::string& fileName, const glm::mat4& transform,
			const Ref<Texture2D>& tex_ptr, const glm::vec4& tintColor=glm::vec4(1.0f), float tiling = 1.0f);

	private:
		struct Renderer_comp_data {
			Ref<VertexArray> vertexArray_ptr;
			Ref<Shader> shader_ptr;
			Ref<Texture2D> white_tex_ptr;
		};
		static Scope<Renderer_comp_data> pm_renderer_comp_data;
		static ShaderLib pm_ShLib;
	};
}
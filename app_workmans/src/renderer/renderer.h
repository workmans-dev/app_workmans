#pragma once
#include "renderer/rendererCommand.h"

namespace AppWMs {

	class Renderer {
	public:

		static void beginScene();
		static void submit(const std::shared_ptr<VertexArray>& vertexArray);
		static void end();

		inline static RendererAPI::SelectRenderer getRenderer() { return RendererAPI::getRenderer(); }

	};
}
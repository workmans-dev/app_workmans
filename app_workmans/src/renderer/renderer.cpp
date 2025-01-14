#include "pch.h"
#include "renderer/renderer.h"


namespace AppWMs {
	void Renderer::beginScene()
	{
	}
	void Renderer::submit(const std::shared_ptr<VertexArray>& vertexArray)
	{
		vertexArray->bind();
		RendererCommand::drawIndexed(vertexArray);
	}
	void Renderer::end()
	{
	}
}
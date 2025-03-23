#include "pch.h"
#include "openGL/openGLRenderer.h"
#include <glad/glad.h>

namespace AppWMs {
	void OpenGLRendererAPI::init()
	{
		APP_WMS_PROFILE_FUNC();
		glEnable(GL_BLEND);
		//blending function with alpha! 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glDepthMask(GL_TRUE);
	}
	void OpenGLRendererAPI::set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
	{
		glViewport(x, y, w, h);
	}
	void OpenGLRendererAPI::setClearColor(const glm::vec4& color){
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::clear(){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount){
		vertexArray->bind();
		uint32_t count = indexCount ? indexCount : vertexArray->getIndexBuffer()->getCount();
		//APP_WMS_CORE_WARN("{0} <---quadIndexCount_in_flush", indexCount)

		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLRendererAPI::drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::setLineWidth(float width)
	{
		glLineWidth(width);
	}
}



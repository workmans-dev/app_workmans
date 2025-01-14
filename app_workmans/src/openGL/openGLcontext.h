#pragma once
#include "renderer/graphicContext.h"

struct GLFWwindow;

namespace AppWMs {
	class OpenGLContext : public GraphicContext {
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual void init() override;
		virtual void swapBuffers() override;
	private:
		GLFWwindow* m_windowHandle;
	};
}
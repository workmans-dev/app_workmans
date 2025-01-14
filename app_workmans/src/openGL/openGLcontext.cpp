#include "pch.h"

#include "openGL/openGLcontext.h"

#include <GLFW/glfw3.h>

//Graphics Library Framework.
//The name reflects its purpose : providing a framework for creating and managing windows, 
//OpenGL / Vulkan contexts, and handling input in graphics applications.Despite the name, 
//GLFW does not include a rendering library; instead, it complements graphics APIs like OpenGL, Vulkan, 
//or OpenGL ES by managing the underlying windowing and input systems.

#include <glad/glad.h>

namespace AppWMs {
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle):m_windowHandle(windowHandle)
	{
		APP_WMS_CORE_ASSERTS(windowHandle, "Window handle(GLFWwindow pointer) is null !!")
	}

	void OpenGLContext::init()
	{
		glfwMakeContextCurrent(m_windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		APP_WMS_CORE_ASSERTS(status, "Failed init!!!!!!");

		// openGL info context.
		APP_WMS_CORE_INFO("openGL Info -------------");
		const GLubyte* glString = glGetString(GL_RENDERER); // Example
		const GLubyte* glVersion = glGetString(GL_VERSION); // Example
		APP_WMS_CORE_INFO("  renderer : {0}", reinterpret_cast<const char*>(glString));
		APP_WMS_CORE_INFO("  version : {0}", reinterpret_cast<const char*>(glVersion));

	}

	void OpenGLContext::swapBuffers()
	{

		glfwSwapBuffers(m_windowHandle);

	}
}
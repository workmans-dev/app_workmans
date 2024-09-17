#include "pch.h"
#include "win_window.h"

#include "events/applicationEvent.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"

namespace AppWMs {

	bool static s_glf_initialized = false;

	Window* Window::create(const WindowProps& Win_props)
	{
		return new Win_Window(Win_props);
	}
	Win_Window::Win_Window(const WindowProps& Win_props) 
	{
		init(Win_props);
	}
	Win_Window::~Win_Window() 
	{
		close();
	}
	void Win_Window::init(const WindowProps& Win_props)
	{
		pm_data.w = Win_props.w;
		pm_data.h = Win_props.h;
		pm_data.title = Win_props.title;
		APP_WMS_CORE_INFO("creating window :  {0}, {1}, {2}",
		Win_props.title, Win_props.w, Win_props.h);
		if (!s_glf_initialized) 
		{
			int success = glfwInit();
			APP_WMS_CORE_ASSERTS(success, "Could not initialze GLFW.");
			s_glf_initialized = true;
		}
		pm_GLFWwindow_ptr = glfwCreateWindow((int)Win_props.w, 
			(int)Win_props.h, Win_props.title.c_str(), 
			nullptr, nullptr);
		glfwMakeContextCurrent(pm_GLFWwindow_ptr);
		glfwSetWindowUserPointer(pm_GLFWwindow_ptr, &pm_data);

		// set GLFW call back
		glfwSetWindowSizeCallback(pm_GLFWwindow_ptr,
			[](GLFWwindow* window, int width, int height) 
			{
				// set construction of the WindowData from Win_Window
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.w = width;
				data.h = height;
				// create WindowResizeEvent from applicationEvent.h
				WindowResizeEvent event(width, height);
				data.EventCallBack(event);
			});
		glfwSetWindowCloseCallback(pm_GLFWwindow_ptr,
			[](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallBack(event);
			});
	}
	void Win_Window::close() 
	{
		glfwDestroyWindow(pm_GLFWwindow_ptr);
	}
	void Win_Window::onUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(pm_GLFWwindow_ptr);
	}
	void Win_Window::setVSync(bool enabled)
	{
		if (enabled) {
			glfwSwapInterval(1);
		}
		else {
			glfwSwapInterval(0);
		}
		pm_data.VSync = enabled;
	}
	bool Win_Window::isVSync() const
	{
		return pm_data.VSync;
	}

}
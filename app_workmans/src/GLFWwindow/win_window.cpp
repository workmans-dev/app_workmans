#include "pch.h"
#include <GLFW/glfw3.h>

#include "win_window.h"

#include "events/applicationEvent.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "vulkan/vulkanContext.h"

namespace AppWMs {

	bool static s_glf_initialized = false;
	static void GLFWErrorCallback(int error, const char* description) {
		APP_WMS_CORE_ERROR("{0}:, {1}", error, description)
	};

	Window* Window::create(const WindowProps& Win_props)
	{
		return new Win_Window(Win_props);
	}

	Win_Window::Win_Window(const WindowProps& Win_props) 
	{
		APP_WMS_PROFILE_FUNC()
		init(Win_props);
	}

	Win_Window::~Win_Window() 
	{
		APP_WMS_PROFILE_FUNC();
		close();
	}

	void Win_Window::init(const WindowProps& Win_props)
	{
		APP_WMS_PROFILE_FUNC()
		m_data.w = Win_props.w;
		m_data.h = Win_props.h;
		m_data.title = Win_props.title;
		APP_WMS_CORE_INFO("creating window :  {0}, {1}, {2}",
		Win_props.title, Win_props.w, Win_props.h);
		if (!s_glf_initialized) 
		{
			int success = glfwInit();
			APP_WMS_CORE_ASSERTS(success, "Could not initialze GLFW.");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_glf_initialized = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_GLFWwindow_ptr = glfwCreateWindow((int)Win_props.w,
			(int)Win_props.h, Win_props.title.c_str(), nullptr, nullptr);
		m_context_ptr = new VulkanContext(m_GLFWwindow_ptr);
		m_context_ptr->init();


		// put WindowData into pm_GLFWWindow_ptr!!!
		glfwSetWindowUserPointer(m_GLFWwindow_ptr, &m_data);
		setVSync(true);

		// set GLFW call back
		glfwSetWindowSizeCallback(m_GLFWwindow_ptr,
			[](GLFWwindow* window, int width, int height) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.w = width;
				data.h = height;
				WindowResizeEvent event(width, height);
				data.EventCallBack(event);
			});
		glfwSetWindowCloseCallback(m_GLFWwindow_ptr,
			[](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallBack(event);
			});
		glfwSetKeyCallback(m_GLFWwindow_ptr,
			[](GLFWwindow* window, int key, int scancode, int action, int mods) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				switch (action) {
					case GLFW_PRESS:{
						KeyPressedEvent event(key, 0);
						data.EventCallBack(event);
						break;
						}
					case GLFW_RELEASE: {
						KeyReleasedEvent event(key);
						data.EventCallBack(event);
						break;
						}
					case GLFW_REPEAT: {
						KeyPressedEvent event(key, 1);
						data.EventCallBack(event);
						break;
						}
				}
			});
		glfwSetCharCallback(m_GLFWwindow_ptr,
			[](GLFWwindow* window, unsigned int key) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(key);
				data.EventCallBack(event);
			});
		glfwSetMouseButtonCallback(m_GLFWwindow_ptr,
			[](GLFWwindow* window, int button, int action, int mods) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				switch (action) {
					case GLFW_PRESS: {
						MousePressedEvent event(button);
						data.EventCallBack(event);
						break;
					}
					case GLFW_RELEASE: {
						MouseReleasedEvent event(button);
						data.EventCallBack(event);
						break;
					}
				}			
			});
		glfwSetScrollCallback(m_GLFWwindow_ptr,
			[](GLFWwindow* window, double xOffset, double yOffset) {
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseScrollEvent event((float)xOffset, (float)yOffset);
				data.EventCallBack(event);
			});
		glfwSetCursorPosCallback(m_GLFWwindow_ptr,
			[](GLFWwindow* window, double xPos, double yPos) {
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallBack(event);
			});
	}

	void Win_Window::close() 
	{
		glfwDestroyWindow(m_GLFWwindow_ptr);
	}

	void Win_Window::onUpdate()
	{
		APP_WMS_PROFILE_FUNC();
		glfwPollEvents();
	}

	inline void Win_Window::setData_EventCallBack(
		const EventCallBackFn& callback)
	{ m_data.EventCallBack = callback; }

	void Win_Window::setVSync(bool enabled)
	{
		APP_WMS_PROFILE_FUNC();
		if (enabled) {
			glfwSwapInterval(1);
		}
		else {
			glfwSwapInterval(0);
		}
		m_data.VSync = enabled;
	}

	bool Win_Window::isVSync() const
	{
		return m_data.VSync;
	}

}
#include "pch.h"
#include "GLFWwindow/windowInput.h"
#include "core/application.h"

#include <GLFW/glfw3.h>

namespace AppWMs {
	Input* Input::m_Input_instance_ptr = new WindowInput();

	bool WindowInput::IsKeyPressedImpl(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(App::getApp().getWindow().getGLFWWindow());
		// get keycode from glfw
		int state = glfwGetKey(window, keycode);
		
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowInput::IsMousePressedImpl(int mouseButton)
	{
		auto window = static_cast<GLFWwindow*>(App::getApp().getWindow().getGLFWWindow());
		int state = glfwGetMouseButton(window, mouseButton);

		return state == GLFW_PRESS ;
	}

	std::pair<float, float> WindowInput::getMousePosImpl()
	{
		auto window = static_cast<GLFWwindow*>(App::getApp().getWindow().getGLFWWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { (float)xPos, (float)yPos };
	}


	float WindowInput::getMouseXImpl()
	{
		auto [x, y] = getMousePosImpl();
		return x;
	}
	float WindowInput::getMouseYImpl()
	{
		auto [x, y] = getMousePosImpl();
		return y;
	}



}
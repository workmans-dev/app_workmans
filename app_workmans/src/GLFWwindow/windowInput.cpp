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


		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
			if (!capture) {
				first_pressed_x = getMouseX();
				first_pressed_y = getMouseY();
				capture = true;
			}
			//first_pressed_x = getMouseX() - first_pressed_x; // Sensitivity factor
			//first_pressed_y = getMouseY() - first_pressed_y;  // Invert Y to match screen space
		}
		else {
			capture = false;
		}

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


	float WindowInput::getMouseDistXImpl()
	{
		return first_pressed_x;
	}

	float WindowInput::getMouseDistYImpl()
	{
		return first_pressed_y;

	}

	std::pair<float, float> WindowInput::getDist_mouseMovedImpl()
	{
		auto window = static_cast<GLFWwindow*>(App::getApp().getWindow().getGLFWWindow());
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		float r = (float)w / float(h);
		float delta_x = (getMouseX() - getMouseDistXImpl())* 0.01f;
		float delta_y = (getMouseY() - getMouseDistYImpl())*  0.01f;
		return { -delta_x, delta_y };
	}
}
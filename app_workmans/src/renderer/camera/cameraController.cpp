#include "pch.h"
#include "core/core.h"
#include "renderer/camera/cameraController.h"
#include "scene/component.h"

namespace AppWMs {

	void CameraController::onCreate()
	{
		//auto& transform = getComponent<TransformComponent>().transform;


	}

	void CameraController::onDestroy()
	{
	}


	void CameraController::onUpdate(Timer ts)
	{
		if (getComponent<CameraMatrixComponent>().primary) {
			auto& translate = getComponent<TransformComponent>().translate;
			if (Input::IsKeyPressed(WMS_KEY_LEFT_ALT)
				&& Input::IsMousePressed(WMS_MOUSE_BUTTON_MIDDLE)) 
			{
				auto [x, y] = Input::getDist_mouseMoved();
				if (x != 0.0f || y != 0.0f) {
					translate.x += (x - last_x);
					translate.y += (y - last_y);
					last_x = x;
					last_y = y;
				}
			}
			else {last_x = 0.0f, last_y = 0.0f;}

			if (Input::IsKeyPressed(WMS_KEY_LEFT_ALT)
				&& Input::IsMousePressed(WMS_MOUSE_BUTTON_LEFT)) {
				pm_aim_vec3.x += Input::getMouseX() * ts;
				pm_aim_vec3.y += Input::getMouseY() * ts;
			}
		}

	}

	void CameraController::onEvent(Event& e)
	{
		//EventDispatcher dispatcher(e);
		//dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(CameraController::resizeWindow));
		//dispatcher.Dispatch<MouseScrollEvent>(BIND_EVENT_FN(CameraController::mouseScroll));
	}


}
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
			//xx += 1.0f;
			//yy += 1.0f;
			//APP_WMS_CORE_INFO("x:{0},y:{1}", xx, yy);
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

			if (pm_rotation) {
				if (Input::IsKeyPressed(WMS_KEY_Q)){
					pm_rotate_vec3.x += pm_rotateSpeed * ts;
				}
				if (Input::IsKeyPressed(WMS_KEY_E)) {
					pm_rotate_vec3.x -= pm_rotateSpeed * ts;
				}

			}
		}


	}

	void CameraController::onEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(CameraController::resizeWindow));
		dispatcher.Dispatch<MouseScrollEvent>(BIND_EVENT_FN(CameraController::mouseScroll));

	}

	bool CameraController::resizeWindow(WindowResizeEvent& e)
	{
		//pm_aspectRatio = (float)e.getWidth() / (float)e.getHeight();
		//pm_camera.setProjection(-pm_aspectRatio * pm_zoomLevel, pm_aspectRatio * pm_zoomLevel,
		//	-pm_zoomLevel, pm_zoomLevel);
		resizeRatio(e.getWidth(), e.getHeight());
		return true;
	}

	bool CameraController::mouseScroll(MouseScrollEvent& e)
	{
		pm_zoomLevel -= e.get_yScroll()*0.5f;
		//pm_camera.setProjection(-pm_aspectRatio * pm_zoomLevel, pm_aspectRatio * pm_zoomLevel,
		//	-pm_zoomLevel, pm_zoomLevel);
		return true;
	}
	void CameraController::resizeRatio(uint32_t w, uint32_t h)
	{
		pm_aspectRatio = (float)w / (float)h;
		//pm_camera.setProjection(-pm_aspectRatio * pm_zoomLevel, pm_aspectRatio * pm_zoomLevel,
		//	-pm_zoomLevel, pm_zoomLevel);
	}
}
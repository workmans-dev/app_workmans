#pragma once
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "events/keyCodes.h"
#include "events/mouseCodes.h"

#include "core/timer.h"
#include "core/input.h"

#include "scene/scriptable_entity.h"


namespace AppWMs {
	class CameraController:public ScriptableEntity {
	public:
		void onCreate();
		void onDestroy();
		void onUpdate(Timer ts);
		void onEvent(Event& e);
		bool resizeWindow(WindowResizeEvent& e);
		bool mouseScroll(MouseScrollEvent& e);
		void resizeRatio(uint32_t w, uint32_t h);


	private:
		float pm_transSpeed=0.0001f;
		float pm_rotateSpeed= 1.0f;
		float pm_aspectRatio = 1.0f;
		float pm_zoomLevel = 1.0f;
		bool pm_rotation = false;
		glm::vec3 pm_translate_vec3 = {0.0f, 0.0f , 0.0f };
		glm::vec3 pm_rotate_vec3 = { 0.0f, 0.0f , 0.0f };
		glm::vec3 pm_aim_vec3 = { 0.0f, 0.0f , 1.0f };
		float last_x=0.0f, last_y = 0.0f;
		bool firstMove = true;
		float xx = 0.0f, yy = 0.0f;
	};
}
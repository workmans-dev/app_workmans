#pragma once
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "events/keyCodes.h"
#include "events/mouseCodes.h"

#include "core/timer.h"
#include "core/input.h"
//#include "scene/scriptable_entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>


namespace AppWMs {
	//class CameraController:public ScriptableEntity {
	class CameraController {

	public:

		// it is not part of ScriptableEntity,
		// if you want to bind this into ScriptableEntity, remove base construct.
		CameraController()=default;
		//
		virtual ~CameraController()=default;
		const glm::mat4& get_projection() const { return m_proj_mat; }
		//void bind() { std::cout << "from Camera class" << std::endl; }



		void onCreate();
		void onDestroy();
		void onUpdate(Timer ts,  glm::vec3& trans,  glm::vec3& rot);
		void onEvent(Event& e);
		bool mouseScroll(MouseScrollEvent& e);


		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;
		void MousePan(const float& delta_x,const float& delta_y, glm::vec3& trans);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(const float& delta);
		std::pair<float, float> PanSpeed()const;
		void UpdateView(glm::vec3& trans, glm::vec3& rot);
		float RotationSpeed() const;
		float ZoomSpeed() const;
		glm::vec3 CalculatePosition() const;

	protected:
		float last_x = 0.0f, last_y = 0.0f, last_z = 0.0f;
		float lastR_x = 0.0f, lastR_y = 0.0f, lastR_z = 0.0f;
		bool firstMove = true;
		float xx = 0.0f, yy = 0.0f;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
		bool m_HasSetFocalPoint = false;
		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f, m_Roll = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
		glm::mat4 m_proj_mat;
	};
}
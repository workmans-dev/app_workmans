#pragma once
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "core/keyCode.h"
#include "core/mouseCode.h"
#include "core/input/keyMap.h"

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
		// WARNING: why there are two projections -- the consumers require different NDC conventions.
		//    Do not try to unify them. It was unified once and the gizmo moved backwards.
		//
		//  get_projection()     the glm original. OpenGL NDC (Y up).
		//                       -> for CPU-side libraries such as ImGuizmo.
		//                         ImGuizmo flips Y itself in worldToPos with
		//                         trans.y = 1 - trans.y (ImGuizmo.cpp:819). Handing it a matrix
		//                         that is already flipped flips it twice and reverses the direction.
		//
		//  get_projection_vk()  a copy with Y flipped. Vulkan NDC (Y down).
		//                       -> for the render pipeline. Without it the image is upside down.
		//                         That flip also reverses the on-screen winding, which is why
		//                         PipelineSpec's frontFace default (CCW) is tied to this.
		const glm::mat4& get_projection() const { return m_proj_mat; }

		glm::mat4 get_projection_vk() const {
			glm::mat4 proj = m_proj_mat;
			proj[1][1] *= -1.0f;
			return proj;
		}
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
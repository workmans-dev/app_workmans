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
		// ⚠️ 투영이 두 벌인 이유 — 소비자가 요구하는 NDC 규약이 다르다.
		//    통합하려 들지 말 것. 한 번 통합했다가 기즈모가 반대로 움직였다.
		//
		//  get_projection()     glm 원본. OpenGL NDC(Y 위).
		//                       → ImGuizmo 같은 CPU측 라이브러리용.
		//                         ImGuizmo는 worldToPos에서 `trans.y = 1 - trans.y`로
		//                         Y를 스스로 뒤집는다(ImGuizmo.cpp:819). 여기에 이미
		//                         뒤집힌 행렬을 주면 두 번 뒤집혀 방향이 반대가 된다.
		//
		//  get_projection_vk()  Y를 뒤집은 사본. 벌칸 NDC(Y 아래).
		//                       → 렌더 파이프라인용. 이걸 안 쓰면 화면이 상하 반전된다.
		//                         이 반전 때문에 화면상 감기도 뒤집히므로
		//                         PipelineSpec의 frontFace 기본값(CCW)이 여기에 묶여 있다.
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
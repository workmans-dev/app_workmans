#include "pch.h"
#include "core/core.h"
#include "renderer/camera/cameraController.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace AppWMs {

	void CameraController::onCreate()
	{
		//auto& transform = getComponent<TransformComponent>().transform;


	}

	void CameraController::onDestroy()
	{
	}

	void CameraController::onUpdate(Timer ts, glm::vec3& trans, glm::vec3& rot)
	{
		// 마야 규약 — modifier를 누른 채로 마우스 버튼을 끈다.
		// 어떤 키/버튼인지는 assets/config/keymap.yaml의 camera 그룹이 정한다.
		if (KeyMap::isHeld("camera.modifier"))
		{

			//auto [x, y] = Input::getDist_mouseMoved();
			//glm::vec2 delta = { x, y };
			const glm::vec2& mouse{ Input::getMouseX(), Input::getMouseY() };
			glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;
			if (KeyMap::isHeld("camera.track")) {
				MousePan(delta.x, delta.y, trans);

			}
			else if (KeyMap::isHeld("camera.tumble")) {
				MouseRotate(delta);

			}

			else if (KeyMap::isHeld("camera.dolly")) {
				MouseZoom(delta.y);

			}


		}
		UpdateView(trans, rot);

		//last_x = 0.0f, last_y = 0.0f;
	}




	void CameraController::MousePan(const float& delta_x, const float& delta_y, glm::vec3& trans)
	{

		auto [xSpeed, ySpeed] = PanSpeed();

		glm::vec3 right = GetRightDirection();
		glm::vec3 up = GetUpDirection();

		m_FocalPoint -= right * delta_x * xSpeed * m_Distance;
		m_FocalPoint += up * delta_y * ySpeed * m_Distance;

		m_Position = CalculatePosition();
	}


	void CameraController::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();

		// ⚠️ 클램프 없음 — 상하 무한 회전(마야 tumble 방식).
		//
		//    m_Pitch는 라디안이다(GetOrientation()이 glm::quat(vec3(...))에 그대로 넣고
		//    그 생성자가 라디안을 받는다). 예전 코드는 ±89로 클램프했는데 그건 도 단위라
		//    ±5100°에 해당해 아무것도 막지 못했다 — 즉 원래도 무한 회전이었고,
		//    클램프가 있다고 착각만 하고 있었다.
		//
		//    극점을 넘으면 카메라가 뒤집히고 GetUpDirection().y가 음수가 된다.
		//    위 yawSign이 그때 좌우 입력을 반전시켜 조작감을 유지한다 — 마야와 같은 처리다.
		//
		//    이 무한 누적이 안전한 이유: quat(vec3(-pitch, -yaw, 0))은 각도가 아무리 커도
		//    잘 정의된다. 예전에 터졌던 건 누적이 아니라 glm::eulerAngles로 되뽑던 쪽이었다.
		m_Position = CalculatePosition();
	}

	void CameraController::MouseZoom(const float& delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			//m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;  // Minimum zoom distance

		}
		m_Position = CalculatePosition();
	}

	void CameraController::UpdateView(glm::vec3& trans, glm::vec3& rot)
	{
		m_Position = CalculatePosition();

		// ⚠️ 예전엔 여기서 GetOrientation()으로 쿼터니언을 만든 뒤
		//    glm::eulerAngles()로 다시 오일러를 뽑아 델타 누적했다. 그게 버그였다.
		//
		//    glm의 eulerAngles는 yaw를 asin으로 구한다(gtc/quaternion.inl:41).
		//    치역이 ±90°라, 카메라 yaw가 그걸 넘는 순간 같은 회전을 다른 조합으로
		//    재표현한다 — pitch와 roll을 각각 180° 뒤집어서. roll이 Z축이므로
		//    화면에선 "Z축 기준 플리핑"으로 보였다. 짐벌락처럼 보이지만 실제로는
		//    쿼터니언→오일러 추출의 분기 문제다(쿼터니언 자체는 멀쩡했다).
		//
		//    게다가 `+=` 델타 누적이라 한순간의 180° 점프가 영구히 남았다.
		//
		//    해법: 추출을 아예 하지 않는다. 컨트롤러가 이미 pitch/yaw를 알고 있으니
		//    그대로 넘긴다. TransformComponent::getLocalMatrix()가 이 값을
		//    glm::quat(glm::radians(rotate))로 되돌리는데, 그게 곧 GetOrientation()과
		//    같은 쿼터니언이라 왕복 손실이 0이다.
		trans = m_Position;
		rot = glm::degrees(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	std::pair<float, float> CameraController::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	void CameraController::onEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		//dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(CameraController::resizeWindow));
		dispatcher.Dispatch<MouseScrollEvent>(BIND_EVENT_FN(CameraController::mouseScroll));
	}



	bool CameraController::mouseScroll(MouseScrollEvent& e)
	{
		//float delta = e.get_yScroll() * 0.1f;
		//auto& transComp = pm_cam_entity->getComponent<TransformComponent>();

		//const glm::mat4& scoll_mat = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, delta });
		//glm::mat4 final_mat = scoll_mat * transComp.getTransform();
		//std::cout << delta << std::endl;
		//transComp.setTransform(final_mat);
		
		//UpdateView();
		return true;
	}

	glm::vec3 CameraController::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 CameraController::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	glm::quat CameraController::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}



	glm::vec3 CameraController::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}




	float CameraController::RotationSpeed() const
	{
		return 0.8f;
	}

	float CameraController::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 50.0f); // max speed = 100
		return speed;
	}

	glm::vec3 CameraController::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}
}
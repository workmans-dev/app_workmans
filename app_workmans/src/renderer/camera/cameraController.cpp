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
		if (Input::IsKeyPressed(WMS_KEY_LEFT_ALT))
		{

			//auto [x, y] = Input::getDist_mouseMoved();
			//glm::vec2 delta = { x, y };
			const glm::vec2& mouse{ Input::getMouseX(), Input::getMouseY() };
			glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;
			if (Input::IsMousePressed(WMS_MOUSE_BUTTON_MIDDLE)) {
				MousePan(delta.x, delta.y, trans);

			}
			else if (Input::IsMousePressed(WMS_MOUSE_BUTTON_LEFT)) {
				MouseRotate(delta);

			}

			else if (Input::IsMousePressed(WMS_MOUSE_BUTTON_RIGHT)) {
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
		m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
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
		// Ensure focal point is correctly initialized (object center)

		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		glm::vec3 rotate = glm::degrees(glm::eulerAngles(orientation));

		trans += (m_Position - glm::vec3(last_x, last_y, last_z));

		last_x = m_Position.x;
		last_y = m_Position.y;
		last_z = m_Position.z;

		rot += (rotate - glm::vec3(lastR_x, lastR_y, lastR_z));

		lastR_x = rotate.x;
		lastR_y	= rotate.y;
		lastR_z	= rotate.z;


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
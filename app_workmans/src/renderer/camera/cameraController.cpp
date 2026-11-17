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
		// Maya convention -- drag a mouse button while holding a modifier.
		// Which key/button that is comes from the camera group in assets/config/keymap.yaml.
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

		// WARNING: no clamp -- unlimited vertical rotation (the Maya tumble style).
		//
		//    m_Pitch is in radians (GetOrientation() feeds it straight into glm::quat(vec3(...)),
		//    whose constructor takes radians). The old code clamped at +/-89, but that was in
		//    degrees, equivalent to +/-5100 degrees, and blocked nothing -- so rotation was always
		//    unlimited, and the clamp was only an illusion.
		//
		//    Past the pole the camera flips over and GetUpDirection().y goes negative.
		//    The yawSign above then inverts horizontal input to keep the feel -- same as Maya.
		//
		//    Why this unbounded accumulation is safe: quat(vec3(-pitch, -yaw, 0)) is well defined
		//    at any angle. What used to break was not the accumulation but the re-extraction with glm::eulerAngles.
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

		// WARNING: this used to build a quaternion with GetOrientation() here and then pull Euler
		//    angles back out with glm::eulerAngles() to accumulate a delta. That was the bug.
		//
		//    glm's eulerAngles derives yaw with asin (gtc/quaternion.inl:41).
		//    Its range is +/-90 degrees, so the moment the camera yaw passes it, the same rotation
		//    is re-expressed as a different combination -- pitch and roll each flipped by 180.
		//    Roll is the Z axis, so on screen it looked like "flipping about Z". It resembles gimbal
		//    lock, but it is really a branch problem in quaternion->Euler extraction (the quaternion itself was fine).
		//
		//    On top of that it was a += delta accumulation, so one momentary 180-degree jump stayed forever.
		//
		//    The fix: do not extract at all. The controller already knows pitch/yaw, so it passes
		//    them straight through. TransformComponent::getLocalMatrix() turns the value back into
		//    glm::quat(glm::radians(rotate)), which is the same quaternion GetOrientation() makes,
		//    so the round-trip loss is zero.
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
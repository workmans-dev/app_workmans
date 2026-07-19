#pragma once
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "events/keyCodes.h"
#include "events/mouseCodes.h"

#include "core/timer.h"
#include <glm/gtc/matrix_transform.hpp>



namespace AppWMs {


	class Camera {
	public:
		Camera()=default;
		virtual ~Camera() = default;
		Camera(const glm::mat4& projection) :m_proj_mat(projection) {}
		const glm::mat4& get_projection() const { return m_proj_mat; }
		//void bind() { std::cout << "from Camera class" << std::endl; }
	protected:
		glm::mat4 m_proj_mat;

	};

}
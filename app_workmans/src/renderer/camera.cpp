#include "pch.h"
#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
namespace AppWMs {

	Camera::Camera(float left, float right, float bottom, float top) :
		pm_proj_mat(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), pm_view_mat(1.0f)
	{
		pm_view_proj_mat = pm_proj_mat * pm_view_mat;
	}

	void Camera::setPosition(const glm::vec3& pos)
	{
		pm_pos = pos;
		reCalulationMat();
	}

	void Camera::setRotation(float rot)
	{
		pm_rot = rot;
		reCalulationMat();
	}

	void Camera::reCalulationMat()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pm_pos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(pm_rot), glm::vec3(0,0,1));
		// Not sure, but invere is necessary?? I don't like unrealEngine camera contrl, should be like
		// Maya, it may be deleted the inverse to make Maya style.
		pm_view_mat = glm::inverse(transform);
		pm_view_proj_mat = pm_proj_mat * pm_view_mat;
	}


}
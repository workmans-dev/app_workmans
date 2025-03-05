#include "pch.h"

#include "renderer/camera/sceneCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace AppWMs {
	SceneCamera::SceneCamera()
	{
		reset_proj_mat();
	}

	void SceneCamera::set_orthographic(const float& size, const float& nearClip, const float& farClip)
	{
		pm_camType = SceneCamera::CameraProjType::orthographic;
		pm_ortho_size = size;
		pm_ortho_nearClip = nearClip;
		pm_ortho_farClip = farClip;
		reset_proj_mat();
	}
	void SceneCamera::set_persp(const float& persp_FOV, const float& nearClip, const float& farClip)
	{
		pm_camType = SceneCamera::CameraProjType::perspective;
		pm_persp_FOV = persp_FOV;
		pm_persp_nearClip = nearClip;
		pm_persp_farClip = farClip;
		reset_proj_mat();
	}
	void SceneCamera::set_proj_mat(const uint32_t& w, const uint32_t& h)
	{
		pm_aspectRatio = (float)w / (float)h;
		reset_proj_mat();
	}

	void SceneCamera::reset_proj_mat()
	{
		if (pm_camType == SceneCamera::CameraProjType::perspective) {
			m_proj_mat = glm::perspective(pm_persp_FOV, pm_aspectRatio, pm_persp_nearClip, pm_persp_farClip);
		}
		else {
			float left = -pm_ortho_size * pm_aspectRatio * 0.5f;
			float right = pm_ortho_size * pm_aspectRatio * 0.5f;
			float bottom = -pm_ortho_size * 0.5f;
			float top = pm_ortho_size * 0.5f;
			// reset projection
			m_proj_mat = glm::ortho(left, right, bottom, top, pm_ortho_nearClip, pm_ortho_farClip);
		}
	}

	
}
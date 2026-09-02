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
		m_ViewportWidth = (float)w, m_ViewportHeight = (float)h;
		pm_aspectRatio = m_ViewportWidth / m_ViewportHeight;
		
		reset_proj_mat();
	}

	void SceneCamera::reset_proj_mat()
	{
		if (pm_camType == SceneCamera::CameraProjType::perspective) {
			m_proj_mat = glm::perspective(pm_persp_FOV, pm_aspectRatio, pm_persp_nearClip, pm_persp_farClip);
			//m_Distance = glm::degrees(pm_persp_FOV);
		}
		else {
			float left = -pm_ortho_size * pm_aspectRatio * 0.5;
			float right = pm_ortho_size * pm_aspectRatio * 0.5f;
			float bottom = -pm_ortho_size * 0.5f;
			float top = pm_ortho_size * 0.5f;
			// reset projection
			m_proj_mat = glm::ortho(left, right, bottom, top, pm_ortho_nearClip, pm_ortho_farClip);
			//m_Distance = pm_ortho_size;

		}

		// ⚠️ m_proj_mat은 glm 원본(OpenGL NDC, Y 위) 그대로 둔다. 여기서 뒤집지 말 것.
		//    벌칸 보정은 get_projection_vk()가 사본에 적용한다 — 이유는 그쪽 주석 참고.
		//APP_WMS_CORE_INFO("reized:{0}, {1}, {2}", m_proj_mat[3].x, m_proj_mat[3].y, m_proj_mat[3].z);
	}

	void SceneCamera::set_camProjType(const CameraProjType& type)
	{
		pm_camType = type;

	}

	
}
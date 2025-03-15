#pragma once
#include "renderer/camera/camera.h"

namespace AppWMs {

	class SceneCamera :public CameraMatrix {
	public:
		enum class CameraProjType{ perspective = 0, orthographic = 1 };
		// Method to convert CameraProjType enum to string
		static const char* CameraProjTypeToString(CameraProjType type) {
			switch (type) {
			case CameraProjType::perspective: return "perspective";
			case CameraProjType::orthographic: return "orthographic";
			default: return "unknown";
			}
		}
		// Method to return an array of const char* strings for camera types
		//static const std::array<const char*, 5> getCameraProjTypes() {
		//	return { CameraProjTypeToString(CameraProjType::left),
		//			 CameraProjTypeToString(CameraProjType::right),
		//			 CameraProjTypeToString(CameraProjType::bottom),
		//			 CameraProjTypeToString(CameraProjType::top),
		//			 CameraProjTypeToString(CameraProjType::perspective) };
		//}

		SceneCamera();
		virtual ~SceneCamera() = default;

		void set_proj_mat(const uint32_t& w, const uint32_t& h);
		void reset_proj_mat();

		const CameraProjType& get_camProjType() const { return pm_camType; }
		void set_camProjType(const CameraProjType& type) { pm_camType = type; }
		inline std::string get_camProjTypeAsString() { return SceneCamera::CameraProjTypeToString(pm_camType); }
	
	private:
		CameraProjType pm_camType = CameraProjType::perspective;
		float pm_ortho_size = 20.0f;
		float pm_ortho_nearClip = -1.0f;
		float pm_ortho_farClip = 1.0f;

		float pm_aspectRatio = 0.0f;

		float pm_persp_FOV = glm::radians(60.0f);
		float pm_persp_nearClip = 0.1f;
		float pm_persp_farClip = 1000.0f;

	public:
		void set_orthographic(const float& size, const float& nearClip, const float& farClip);
		void set_persp(const float& persp_FOV, const float& nearClip, const float& farClip);

		float get_ortho_size() { return pm_ortho_size; }
		float get_ortho_nearClip() { return pm_ortho_nearClip; }
		float get_ortho_farClip() { return pm_ortho_farClip; }
		void set_ortho_size(const float& size) { pm_ortho_size=size; reset_proj_mat();}
		void set_ortho_nearClip(const float& nearClip) {pm_ortho_nearClip = nearClip; reset_proj_mat();}
		void set_ortho_farClip(const float& farClip) { pm_ortho_farClip = farClip; reset_proj_mat();}

		float get_persp_FOV() { return pm_persp_FOV; }
		float get_persp_nearClip() { return pm_persp_nearClip; }
		float get_persp_farClip() { return pm_persp_farClip; }
		void set_persp_FOV(const float& persp_FOV) { pm_persp_FOV = persp_FOV; reset_proj_mat(); }
		void set_persp_nearClip(const float& nearClip) { pm_persp_nearClip = nearClip; reset_proj_mat();}
		void set_persp_farClip(const float& farClip) { pm_persp_farClip = farClip; reset_proj_mat();}
	};
}
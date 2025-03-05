#pragma once
#include <glm/glm.hpp>

namespace AppWMs {


	class CameraMatrix {
	public:
		CameraMatrix()=default;
		virtual ~CameraMatrix() = default;
		CameraMatrix(const glm::mat4& projection) :m_proj_mat(projection) {}
		const glm::mat4x4& get_projection() const { return m_proj_mat; }
	protected:
		glm::mat4x4 m_proj_mat;

	};

}
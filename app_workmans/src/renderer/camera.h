#pragma once
#include <glm/glm.hpp>

namespace AppWMs {
	class Camera {
	public:
		Camera(float left, float right, float bottom, float top);
		const glm::vec3& getPosition() const { return pm_pos; }
		const float getRotation() const { return pm_rot; }
		void setPosition(const glm::vec3& pos);
		void setRotation(float rot);

		const glm::mat4& get_proj_mat() { return pm_proj_mat; }
		const glm::mat4& get_view_mat() { return pm_view_mat; }
		const glm::mat4& get_pm_view_proj_mat() { return pm_view_proj_mat; }

	private:
		void reCalulationMat();

	private:
		// left. right. bottom, top
		glm::mat4 pm_proj_mat;
		// matrixes
		glm::mat4 pm_view_mat;
		glm::mat4 pm_view_proj_mat;
		glm::vec3 pm_pos = {0.0f,0.0f,0.0f};

		float pm_rot = 0.0f;

	};

}
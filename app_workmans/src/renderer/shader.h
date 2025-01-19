#pragma once
#include <string>
#include <glm/glm.hpp>


namespace AppWMs {

	class Shader {
	public:
		Shader(const std::string& vertexSource, const std::string& fragmentSource);
		~Shader();
		void bind() const;
		void unbind() const;
		void uploadUniformMatrix(const std::string& name, const glm::mat4& mat);
		void uploadUniformFloat4(const std::string& name, const glm::vec4& values);
	private:
		uint32_t pm_renderID;
		
	};
}
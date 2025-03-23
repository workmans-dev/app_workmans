#pragma once
#include <glm/glm.hpp>
#include "renderer/shader.h"

// TODO : remove it!@
typedef unsigned int GLenum;


namespace AppWMs {

	class OpenGLShader:public Shader {
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& fileName, 
			const std::string& vertexSource, const std::string& fragmentSource);
		virtual ~OpenGLShader() override;
		void bind() const override;
		void unbind() const override;
		virtual void uploadUniformMat3(const std::string& name, const glm::mat3& mat) override;
		virtual void uploadUniformMat4(const std::string& name, const glm::mat4& mat)override;
		virtual void uploadUniformInt(const std::string& name, const int& value)override;
		virtual void uploadUniformIntArray(const std::string& name, int* values, uint32_t count)override;
		virtual void uploadUniformFloat(const std::string& name, const float& value)override;
		virtual void uploadUniformFloat2(const std::string& name, const glm::vec2& values)override;
		virtual void uploadUniformFloat3(const std::string& name, const glm::vec3& values)override;
		virtual void uploadUniformFloat4(const std::string& name, const glm::vec4& values)override;

		virtual const std::string& get_fileName() const override { return pm_fileName; }

	private:
		uint32_t pm_renderID = 0;
		std::string pm_fileName;
		std::string pm_filePath;

	private:
		void compile(const std::unordered_map<GLenum, std::string>& preProcess);
		std::unordered_map<GLenum, std::string>preProcess(const std::string& src);
		std::string readFile(const std::string& filePath);

	};
}
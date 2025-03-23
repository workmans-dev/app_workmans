#pragma once
#include <string>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>



namespace AppWMs {

	class Shader {
	public:
		virtual ~Shader() = default;
		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		static Ref<Shader> create(const std::string& filePath);
		static Ref<Shader> create(const std::string& fileName,
			const std::string& vertexSource, const std::string& fragmentSource);
		virtual const std::string& get_fileName() const = 0;

		virtual void uploadUniformMat3(const std::string& name, const glm::mat3& mat) =0;
		virtual void uploadUniformMat4(const std::string& name, const glm::mat4& mat) = 0;
		virtual void uploadUniformInt(const std::string& name, const int& value) = 0;
		virtual void uploadUniformIntArray(const std::string& name, int* values, uint32_t count) =0;
		virtual void uploadUniformFloat(const std::string& name, const float& value) = 0;
		virtual void uploadUniformFloat2(const std::string& name, const glm::vec2& values) = 0;
		virtual void uploadUniformFloat3(const std::string& name, const glm::vec3& values) = 0;
		virtual void uploadUniformFloat4(const std::string& name, const glm::vec4& values) = 0;

	};
	
	class ShaderLib {
	public:
		void add(const Ref<Shader>& shader);
		void addAs(const std::string& custom_fileName, const Ref<Shader>& shader);
		Ref<Shader> load (const std::string& filePath);
		Ref<Shader> load (const std::string& name, const std::string& filePath);
		Ref<Shader> getShader(const std::string& name);
		bool exist_fileName(const std::string& fileName) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> pm_fileName_shader_data;
	};
}
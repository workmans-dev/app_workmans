#include "pch.h"


#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "openGL/openGLShader.h"


namespace AppWMs{
	
	OpenGLShader::OpenGLShader(const std::string& filePath)
	{
		APP_WMS_PROFILE_FUNC();
		std::string str_fromFile = readFile(filePath);
		std::unordered_map<GLenum, std::string> shaderSrc = preProcess(str_fromFile);
		compile(shaderSrc);

		// get fileName from the filePath
		std::size_t lastSlash_index = filePath.find_last_of("/\\");
		lastSlash_index = lastSlash_index == std::string::npos ? 0 : lastSlash_index + 1;
		std::size_t ext_index = filePath.rfind(".");
		std::size_t count = ext_index == std::string::npos ?
			filePath.size() - lastSlash_index : ext_index - lastSlash_index;
		
		pm_fileName = filePath.substr(lastSlash_index, count);
	}
	OpenGLShader::OpenGLShader(const std::string& fileName,
		const std::string& vertexSource, const std::string& fragmentSource):pm_fileName(fileName)
	{
		std::unordered_map<GLenum, std::string> shaderSrc;
		shaderSrc[GL_VERTEX_SHADER] = vertexSource;
		shaderSrc[GL_FRAGMENT_SHADER] = fragmentSource;
		compile(shaderSrc);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(pm_renderID);
	}

	void OpenGLShader::bind() const
	{
		glUseProgram(pm_renderID);
	}

	void OpenGLShader::unbind() const
	{
		glUseProgram(0);
	}
	void OpenGLShader::uploadUniformMat3(const std::string& name, const glm::mat3& mat)
	{
		GLint location = glGetUniformLocation(pm_renderID, name.c_str());
		// f : float, v: vector >> float vector  >> ~fv
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}
	void OpenGLShader::uploadUniformMat4(const std::string& name, const glm::mat4& mat)
	{
		GLint location = glGetUniformLocation(pm_renderID, name.c_str());
		// f : float, v: vector >> float vector  >> ~fv
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void OpenGLShader::uploadUniformInt(const std::string& name, const int& value)
	{
		GLint location = glGetUniformLocation(pm_renderID, name.c_str());
		glUniform1i(location, value);
	}
	void OpenGLShader::uploadUniformFloat(const std::string& name, const float& value)
	{
		GLint location = glGetUniformLocation(pm_renderID, name.c_str());
		glUniform1f(location, value);
	}
	void OpenGLShader::uploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{
		GLint location = glGetUniformLocation(pm_renderID, name.c_str());
		glUniform2f(location, values.x, values.y);
	}
	void OpenGLShader::uploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{
		GLint location = glGetUniformLocation(pm_renderID, name.c_str());
		glUniform3f(location, values.x, values.y, values.z);
	}
	void OpenGLShader::uploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{
		GLint location = glGetUniformLocation(pm_renderID, name.c_str());
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex") {
			return GL_VERTEX_SHADER;
		}
		if (type == "fragment"|| type == "pixel") {
			return GL_FRAGMENT_SHADER;
		}
		return GL_FALSE;
	}

	void OpenGLShader::compile(const std::unordered_map<GLenum, std::string>& preProcess)
	{
		APP_WMS_PROFILE_FUNC();
		GLuint program  = glCreateProgram();
		std::array<GLenum, 2> glShaderIDs;
		int glShaderID_index = 0;
		for (auto& kv : preProcess) {
			GLenum type = kv.first;
			const std::string& src = kv.second;
			// Create an empty vertex shader handle
			GLuint shaderID = glCreateShader(type);
			// Send the vertex shader source code to GL
			// Note that std::string's .c_str is NULL character terminated.
			const GLchar* source = (const GLchar*)src.c_str();
			glShaderSource(shaderID, 1, &source, 0);

			// Compile the vertex shader
			glCompileShader(shaderID);
			GLint isCompiled = 0;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shaderID, maxLength, &maxLength, &infoLog[0]);
				// We don't need the shader anymore.
				glDeleteShader(shaderID);
				APP_WMS_CORE_ERROR("{0}", infoLog.data());
				return;
			}
			// Attach our shaders to our program
			glAttachShader(program, shaderID);
			glShaderIDs[glShaderID_index++] = shaderID;
		}

		// Link our program
		glLinkProgram(program);
		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			// We don't need the program anymore.
			glDeleteProgram(program);
			for (auto id : glShaderIDs)
				// Don't leak shaders either.
				glDeleteShader(program);
			APP_WMS_CORE_ERROR("{0}", infoLog.data());
			return;
		}
		for (auto id : glShaderIDs)
			// Always detach shaders after a successful link.
			glDetachShader(program, id);
		pm_renderID = program;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::preProcess(const std::string& src)
	{
		APP_WMS_PROFILE_FUNC();
		std::unordered_map<GLenum, std::string> shaderSrc;
		const char* typeToken = "#type";
		size_t typeToken_len = strlen(typeToken);
		size_t pos = src.find(typeToken, 0);
		while (pos != std::string::npos) {
			// see the end of the line whether the end is "\r\n" or not. should be like #type vertex
			// not like #type vertex ... something ...
			size_t eol = src.find_first_of("\r\n", pos);
			APP_WMS_CORE_ASSERTS(eol != std::string::npos, "Syntax error");
			// +1 because #type<_>vertex, for this space.
			size_t begin_typeToken = pos + typeToken_len + 1;
			std::string type = src.substr(begin_typeToken, eol - begin_typeToken);
			APP_WMS_CORE_ASSERTS(type=="vertex" || type == "fragment" || type=="pixel",
				"Invalid shader type");

			size_t nexLinePos = src.find_first_not_of("\r\n", eol);
			pos = src.find(typeToken, nexLinePos);
			shaderSrc[ShaderTypeFromString(type)] =
				src.substr(nexLinePos,
					pos - (nexLinePos == std::string::npos ? src.size() - 1 : nexLinePos));

		}
		return shaderSrc;
	}

	std::string OpenGLShader::readFile(const std::string& filePath)
	{
		std::string str_fromFile;
		std::fstream in(filePath, std::ios::in, std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			str_fromFile.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&str_fromFile[0], str_fromFile.size());
			in.close();
		}
		else {
			APP_WMS_CORE_ERROR("Couldn't open the file", filePath);
		}

		return str_fromFile;
	}
}
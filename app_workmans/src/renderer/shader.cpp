#include "pch.h"
#include "shader.h"

#include "renderer/renderer.h"


namespace AppWMs {
	Ref<Shader> Shader::create(const std::string& filePath)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: 
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::Vulkan: 
			return std::make_shared<OpenGLShader>(filePath);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}

	Ref<Shader> Shader::create(const std::string& fileName,
		const std::string& vertexSource, const std::string& fragmentSource)
	{
		switch (Renderer::getRenderer()) {
		case RendererAPI::SelectRenderer::None: 
			APP_WMS_CORE_ASSERTS(false, "Renderer is None");
		case RendererAPI::SelectRenderer::OpenGL: 
			return std::make_shared< OpenGLShader>(fileName,
			vertexSource, fragmentSource);
		}
		APP_WMS_CORE_ASSERTS(false, "Unknown renderer");
		return nullptr;
	}




	// ShaderLib --------------------------------------
	void ShaderLib::add(const Ref<Shader>& shader) {
		const std::string& fileName = shader->get_fileName();
		addAs(fileName, shader);
	}

	void ShaderLib::addAs(const std::string& custom_fileName, const Ref<Shader>& shader)
	{
		APP_WMS_CORE_ASSERTS(!exist_fileName(custom_fileName), "shader file name already exists!");
		pm_fileName_shader_data[custom_fileName] = shader;
	}

	Ref<Shader> ShaderLib::load(const std::string& filePath) {
		Ref<Shader> shader =  Shader::create(filePath);
		add(shader);
		return shader;
	}

	Ref<Shader> ShaderLib::load(const std::string& name, const std::string& filePath) {
		Ref<Shader> shader = Shader::create(filePath);
		addAs(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLib::getShader(const std::string& name) {
		APP_WMS_CORE_ASSERTS(exist_fileName(name), "shader file name doesn't exist!");
		return pm_fileName_shader_data[name];
	}

	bool ShaderLib::exist_fileName(const std::string& fileName) const{
		// pm_fileName_shader_data.find(fileName) is the 
		// std::unordered_map<std::string, std::shared_ptr<Shader>>::iterator

		// if fileName doesn't exist, find("") returns pm_fileName_shader_data.end()
		return pm_fileName_shader_data.find(fileName) != pm_fileName_shader_data.end();
	}
}
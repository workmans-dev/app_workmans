#include "pch.h"
#include "vulkan/vk_shaderCompiler.h"
#include <shaderc/shaderc.hpp>

namespace AppWMs {
	namespace VulkanShaderCompiler {

		static shaderc_shader_kind ToShadercKind(ShaderStage stage) {
			switch (stage) {
			case ShaderStage::Vertex:   return shaderc_glsl_vertex_shader;
			case ShaderStage::Fragment: return shaderc_glsl_fragment_shader;
			case ShaderStage::Compute:  return shaderc_glsl_compute_shader;
			case ShaderStage::Geometry: return shaderc_glsl_geometry_shader;
			}
			APP_WMS_CORE_ASSERTS(false, "Unknown shader stage!")
				return shaderc_glsl_vertex_shader;
		}

		std::vector<uint32_t> CompileGLSL(const std::string& source,
			ShaderStage stage, const std::string& debugName) {

			shaderc::Compiler compiler;
			shaderc::CompileOptions options;

			// 벌칸 1.3 타겟 (인스턴스 apiVersion과 일치)
			options.SetTargetEnvironment(shaderc_target_env_vulkan,
				shaderc_env_version_vulkan_1_3);

#ifdef APP_WMS_DEBUG
			options.SetGenerateDebugInfo();
			options.SetOptimizationLevel(shaderc_optimization_level_zero);
#else
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
			auto result = compiler.CompileGlslToSpv(source,
				ToShadercKind(stage), debugName.c_str(), options);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				APP_WMS_CORE_ERROR("셰이더 컴파일 실패 [{0}]: {1}",
					debugName, result.GetErrorMessage());
				return {};
			}
			return std::vector<uint32_t>(result.cbegin(), result.cend());
		}

		std::vector<uint32_t> CompileFromFile(const std::string& filepath, ShaderStage stage) {
			std::ifstream in(filepath, std::ios::in | std::ios::binary);
			if (!in) {
				APP_WMS_CORE_ERROR("셰이더 파일 열기 실패: {0}", filepath);
				return {};
			}
			std::string source;
			in.seekg(0, std::ios::end);
			source.resize((size_t)in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(source.data(), source.size());
			in.close();

			return CompileGLSL(source, stage, filepath);
		}

		// ⚠️ CreateShaderModule은 vk_shaderModule.cpp로 이전했다.
		//    shaderc에 의존하지 않아 이 파일이 빌드 제외여도 살아 있어야 하기 때문.
	}
}
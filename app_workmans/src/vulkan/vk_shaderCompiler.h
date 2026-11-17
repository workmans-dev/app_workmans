#pragma once
#include <vulkan/vulkan.h>

namespace AppWMs {
	namespace VulkanShaderCompiler {

		enum class ShaderStage { Vertex, Fragment, Compute, Geometry };

		// -- Active path (vk_shaderModule.cpp) ----------------------
		// Reads the .spv that compile_shaders.bat baked at build time. No runtime compilation.
		std::vector<uint32_t> LoadSpirvFile(const std::string& filepath);

		// SPIR-V -> VkShaderModule
		VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& spirv);

		// -- Dormant path (vk_shaderCompile.cpp -- excluded from the build) --
		// shaderc_combined.lib ships only as /MD, which collides with /MTd at link time.
		// If hot reload becomes necessary, build shaderc from source as /MTd and revive this.
		// Only the declarations remain, so calling these gives a link error.
		std::vector<uint32_t> CompileGLSL(const std::string& source,
			ShaderStage stage, const std::string& debugName = "shader");
		std::vector<uint32_t> CompileFromFile(const std::string& filepath, ShaderStage stage);
	}
}

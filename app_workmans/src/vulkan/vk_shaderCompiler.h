#pragma once
#include <vulkan/vulkan.h>

namespace AppWMs {
    namespace VulkanShaderCompiler {

        enum class ShaderStage { Vertex, Fragment, Compute, Geometry };

        // ── 현재 경로 (vk_shaderModule.cpp) ─────────────────────────
        // 빌드 시 compile_shaders.bat이 구워둔 .spv를 읽는다. 런타임 컴파일 없음.
        std::vector<uint32_t> LoadSpirvFile(const std::string& filepath);

        // SPIR-V → VkShaderModule
        VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& spirv);

        // ── 휴면 경로 (vk_shaderCompile.cpp — 빌드 제외 중) ──────────
        // shaderc_combined.lib이 /MD로만 배포되어 /MTd와 링크 충돌.
        // 핫 리로드가 필요해지면 shaderc를 /MTd로 소스 빌드해 되살릴 것.
        // 선언만 남겨두므로 호출하면 링크 에러가 난다.
        std::vector<uint32_t> CompileGLSL(const std::string& source,
            ShaderStage stage, const std::string& debugName = "shader");
        std::vector<uint32_t> CompileFromFile(const std::string& filepath, ShaderStage stage);
    }
}
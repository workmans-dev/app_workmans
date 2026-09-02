#include "pch.h"
#include "vulkan/vk_shaderCompiler.h"

// shaderc에 의존하지 않는 셰이더 경로.
// GLSL 컴파일은 빌드 시점(compile_shaders.bat + glslc)에 끝나고,
// 런타임은 여기서 .spv 파일을 읽어 모듈로 만들기만 한다.
namespace AppWMs {
    namespace VulkanShaderCompiler {

        std::vector<uint32_t> LoadSpirvFile(const std::string& filepath) {
            std::ifstream in(filepath, std::ios::in | std::ios::binary | std::ios::ate);
            if (!in) {
                APP_WMS_CORE_ERROR("SPIR-V 파일 열기 실패: {0}  (compile_shaders.bat 실행됐는지 확인)",
                    filepath);
                return {};
            }

            const std::streamsize byteSize = in.tellg();
            if (byteSize <= 0 || (byteSize % sizeof(uint32_t)) != 0) {
                // SPIR-V는 32비트 워드 배열이라 4의 배수가 아니면 파일이 깨진 것
                APP_WMS_CORE_ERROR("SPIR-V 크기가 비정상: {0} ({1} bytes)", filepath, (int64_t)byteSize);
                return {};
            }

            std::vector<uint32_t> spirv((size_t)byteSize / sizeof(uint32_t));
            in.seekg(0, std::ios::beg);
            in.read(reinterpret_cast<char*>(spirv.data()), byteSize);
            in.close();

            return spirv;
        }

        VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& spirv) {
            if (spirv.empty()) {
                APP_WMS_CORE_ERROR("빈 SPIR-V로 ShaderModule 생성 시도!");
                return VK_NULL_HANDLE;
            }
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = spirv.size() * sizeof(uint32_t); // ⭐️ 바이트 단위
            createInfo.pCode = spirv.data();

            VkShaderModule shaderModule = VK_NULL_HANDLE;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("ShaderModule 생성 실패!");
                return VK_NULL_HANDLE;
            }
            return shaderModule;
        }
    }
}

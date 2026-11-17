#include "pch.h"
#include "vulkan/vk_shaderCompiler.h"

// The shader path that does not depend on shaderc.
// GLSL compilation finishes at build time (compile_shaders.bat + glslc);
// at runtime this only reads the .spv file and turns it into a module.
namespace AppWMs {
    namespace VulkanShaderCompiler {

        std::vector<uint32_t> LoadSpirvFile(const std::string& filepath) {
            std::ifstream in(filepath, std::ios::in | std::ios::binary | std::ios::ate);
            if (!in) {
                APP_WMS_CORE_ERROR("Failed to open SPIR-V file: {0}  (check that compile_shaders.bat has run)",
                    filepath);
                return {};
            }

            const std::streamsize byteSize = in.tellg();
            if (byteSize <= 0 || (byteSize % sizeof(uint32_t)) != 0) {
                // SPIR-V is an array of 32-bit words, so a size that is not a multiple of 4 means a corrupt file
                APP_WMS_CORE_ERROR("Invalid SPIR-V size: {0} ({1} bytes)", filepath, (int64_t)byteSize);
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
                APP_WMS_CORE_ERROR("Tried to create a ShaderModule from empty SPIR-V!");
                return VK_NULL_HANDLE;
            }
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = spirv.size() * sizeof(uint32_t); // in bytes
            createInfo.pCode = spirv.data();

            VkShaderModule shaderModule = VK_NULL_HANDLE;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("Failed to create ShaderModule!");
                return VK_NULL_HANDLE;
            }
            return shaderModule;
        }
    }
}

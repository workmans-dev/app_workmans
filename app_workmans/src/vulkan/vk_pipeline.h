#pragma once
#include "renderer/buffer.h"
#include <vulkan/vulkan.h>

namespace AppWMs {


	struct PipelineSpec {
		// Path to the .spv baked by compile_shaders.bat at build time (not the GLSL source)
		std::string vertPath;
		std::string fragPath;

		VkRenderPass renderPass = VK_NULL_HANDLE;
		BufferLayout vertexLayout;
		uint32_t pushConstantSize = 0;

		VkShaderStageFlags pushConstantStages = VK_SHADER_STAGE_VERTEX_BIT;

		uint32_t colorAttachmentCount = 1;

		// Render state - a difference here means a separate pipeline even for the same shader
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
		// normal vector - front way.
		VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		bool depthTest = true;
		bool depthWrite = true;
	};

	class VulkanPipeline {
	public:
		VulkanPipeline(const PipelineSpec& spec);
		~VulkanPipeline();

		// VkPipeline is an owned handle, so copying it would double-destroy
		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;

		void bind(VkCommandBuffer cmd) const;
		void pushConstants(VkCommandBuffer cmd, const void* data, uint32_t size) const;

		bool isValid() const { return m_Pipeline != VK_NULL_HANDLE; }
		VkPipeline getHandle() const { return m_Pipeline; }
		VkPipelineLayout getLayout() const { return m_Layout; }

	private:
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_Layout = VK_NULL_HANDLE;
		uint32_t m_PushConstantSize = 0;
		// pushConstants() forwards this straight to vkCmdPushConstants - a mismatch
		// with the layout declaration raises a validation error.
		VkShaderStageFlags m_PushConstantStages = VK_SHADER_STAGE_VERTEX_BIT;
	};
}

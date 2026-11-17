#include "pch.h"
#include "vulkan/vk_pipeline.h"
#include "vulkan/vulkanContext.h"
#include "vulkan/vk_shaderCompiler.h"
#include "vulkan/vk_vertexArray.h"   // VulkanVertexInput::Build
#include "vulkan/vulkanRenderer.h"   // setCurrentPipeline
#include "renderer/rendererCommand.h"

namespace AppWMs {

	VulkanPipeline::VulkanPipeline(const PipelineSpec& spec)
		: m_PushConstantSize(spec.pushConstantSize)
		, m_PushConstantStages(spec.pushConstantStages)
	{
		VkDevice device = VulkanContext::Get()->getDevice();

		if (spec.renderPass == VK_NULL_HANDLE) {
			APP_WMS_CORE_ERROR("Pipeline: no renderPass. FrameBuffer should be the first");
			return;
		}

		// =====================================================================
		// 1. Shaders -- read the .spv baked at build time into two modules.
		//    The modules are only needed to create the pipeline, so they are
		//    destroyed right at the end of this function.
		// =====================================================================
		auto vertSpirv = VulkanShaderCompiler::LoadSpirvFile(spec.vertPath);
		auto fragSpirv = VulkanShaderCompiler::LoadSpirvFile(spec.fragPath);

		VkShaderModule vertModule = VulkanShaderCompiler::CreateShaderModule(device, vertSpirv);
		VkShaderModule fragModule = VulkanShaderCompiler::CreateShaderModule(device, fragSpirv);

		if (vertModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE) {
			APP_WMS_CORE_ERROR("Pipeline: shader module failed -> stopped the pipeline");
			if (vertModule) vkDestroyShaderModule(device, vertModule, nullptr);
			if (fragModule) vkDestroyShaderModule(device, fragModule, nullptr);
			return;
		}

		VkPipelineShaderStageCreateInfo stages[2]{};
		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[0].module = vertModule;
		stages[0].pName = "main";

		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[1].module = fragModule;
		stages[1].pName = "main";

		// =====================================================================
		// 2. Pipeline layout -- the shape of the data the shader takes besides vertices.
		//    The first cube uses push constants only (zero descriptor sets).
		// =====================================================================
		VkPushConstantRange pushRange{};
		pushRange.stageFlags = m_PushConstantStages;
		pushRange.offset = 0;
		pushRange.size = m_PushConstantSize;

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = (m_PushConstantSize > 0) ? 1 : 0;
		layoutInfo.pPushConstantRanges = (m_PushConstantSize > 0) ? &pushRange : nullptr;

		if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_Layout) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("Failed to create PipelineLayout!");
			vkDestroyShaderModule(device, vertModule, nullptr);
			vkDestroyShaderModule(device, fragModule, nullptr);
			return;
		}

		// =====================================================================
		// 3. Vertex input -- the converter from step 3-a takes the BufferLayout as it is
		// =====================================================================
		auto vertexInput = VulkanVertexInput::Build(spec.vertexLayout, 0);

		VkPipelineVertexInputStateCreateInfo viState{};
		viState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		viState.vertexBindingDescriptionCount = (uint32_t)vertexInput.bindings.size();
		viState.pVertexBindingDescriptions = vertexInput.bindings.data();
		viState.vertexAttributeDescriptionCount = (uint32_t)vertexInput.attributes.size();
		viState.pVertexAttributeDescriptions = vertexInput.attributes.data();

		// =====================================================================
		// 4. Fixed state
		// =====================================================================
		VkPipelineInputAssemblyStateCreateInfo ia{};
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		ia.primitiveRestartEnable = VK_FALSE;

		// Viewport/scissor are dynamic, so the pipeline is not rebuilt every time the panel resizes.
		// The actual values are set by VulkanRenderTarget::beginPass() via vkCmdSetViewport/Scissor.
		VkPipelineViewportStateCreateInfo vp{};
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.viewportCount = 1;
		vp.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rs{};
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.depthClampEnable = VK_FALSE;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.polygonMode = spec.polygonMode;
		rs.cullMode = spec.cullMode;
		rs.frontFace = spec.frontFace;
		rs.depthBiasEnable = VK_FALSE;
		rs.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo ms{};
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;   // must match RenderTargetSpec.samples=1
		ms.sampleShadingEnable = VK_FALSE;

		// The depth clear is 1.0 and GLM_FORCE_DEPTH_ZERO_TO_ONE gives 0..1, so LESS is correct
		VkPipelineDepthStencilStateCreateInfo ds{};
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.depthTestEnable = spec.depthTest ? VK_TRUE : VK_FALSE;
		ds.depthWriteEnable = spec.depthWrite ? VK_TRUE : VK_FALSE;
		ds.depthCompareOp = VK_COMPARE_OP_LESS;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.stencilTestEnable = VK_FALSE;

		// The attachment count must equal the colorAttachmentCount of the render pass subpass.
		// WARNING: blendEnable has to be FALSE for all of them -- an integer format such as
		//          the id buffer (R32_SINT) forbids blending outright. To enable blending on
		//          a color slot, the slots have to be split out individually.
		VkPipelineColorBlendAttachmentState blendAttach{};
		blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttach.blendEnable = VK_FALSE;
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(
			spec.colorAttachmentCount, blendAttach);

		VkPipelineColorBlendStateCreateInfo cb{};
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.logicOpEnable = VK_FALSE;
		cb.attachmentCount = (uint32_t)blendAttachments.size();
		cb.pAttachments = blendAttachments.data();

		VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dyn{};
		dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dyn.dynamicStateCount = 2;
		dyn.pDynamicStates = dynStates;

		// =====================================================================
		// 5. Create the pipeline
		// =====================================================================
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = stages;
		pipelineInfo.pVertexInputState = &viState;
		pipelineInfo.pInputAssemblyState = &ia;
		pipelineInfo.pViewportState = &vp;
		pipelineInfo.pRasterizationState = &rs;
		pipelineInfo.pMultisampleState = &ms;
		pipelineInfo.pDepthStencilState = &ds;
		pipelineInfo.pColorBlendState = &cb;
		pipelineInfo.pDynamicState = &dyn;
		pipelineInfo.layout = m_Layout;
		pipelineInfo.renderPass = spec.renderPass;
		pipelineInfo.subpass = 0;

		VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&pipelineInfo, nullptr, &m_Pipeline);

		// The modules are needed only up to here -- the pipeline has already absorbed the code
		vkDestroyShaderModule(device, vertModule, nullptr);
		vkDestroyShaderModule(device, fragModule, nullptr);

		if (result != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("GraphicsPipeline createion failed! (VkResult {0})", (int)result);
			return;
		}

		APP_WMS_CORE_INFO("Pipeline build finished - attr {0}, stride {1}, push {2}B",
			vertexInput.attributes.size(), spec.vertexLayout.getStride(), m_PushConstantSize);
	}

	VulkanPipeline::~VulkanPipeline() {
		VkDevice device = VulkanContext::Get()->getDevice();
		if (m_Pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, m_Pipeline, nullptr);
		}
		if (m_Layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(device, m_Layout, nullptr);
		}
	}

	void VulkanPipeline::bind(VkCommandBuffer cmd) const {
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		// Register ourselves so that pushObjectMatrices can find and use this pipeline's layout.
		// This detour exists because VkPipelineLayout cannot be passed through the abstract API.
		static_cast<VulkanRendererAPI*>(&RendererCommand::getAPI())->setCurrentPipeline(this);
	}

	void VulkanPipeline::pushConstants(VkCommandBuffer cmd, const void* data, uint32_t size) const {
		if (m_PushConstantSize == 0) return;
		// Exceeding the range declared in the layout is a validation error -> clamp to the declared size.
		// stageFlags must also match the declaration exactly, so the value from the spec is used as is.
		vkCmdPushConstants(cmd, m_Layout, m_PushConstantStages, 0,
			(size < m_PushConstantSize) ? size : m_PushConstantSize, data);
	}
}

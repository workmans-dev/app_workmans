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
		// 1. 셰이더 — 빌드 시 구워둔 .spv를 읽어 모듈 2개
		//    모듈은 파이프라인 생성에만 쓰이므로 함수 끝에서 바로 파괴한다.
		// =====================================================================
		auto vertSpirv = VulkanShaderCompiler::LoadSpirvFile(spec.vertPath);
		auto fragSpirv = VulkanShaderCompiler::LoadSpirvFile(spec.fragPath);

		VkShaderModule vertModule = VulkanShaderCompiler::CreateShaderModule(device, vertSpirv);
		VkShaderModule fragModule = VulkanShaderCompiler::CreateShaderModule(device, fragSpirv);

		if (vertModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE) {
			APP_WMS_CORE_ERROR("Pipeline: shader module failed → stopped the pipeline");
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
		// 2. 파이프라인 레이아웃 — 셰이더가 정점 외에 받는 데이터의 형태
		//    첫 큐브는 push constant만 쓴다 (디스크립터셋 0개).
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
			APP_WMS_CORE_ERROR("PipelineLayout 생성 실패!");
			vkDestroyShaderModule(device, vertModule, nullptr);
			vkDestroyShaderModule(device, fragModule, nullptr);
			return;
		}

		// =====================================================================
		// 3. 정점 입력 — 3-a의 변환기가 BufferLayout을 그대로 받아준다
		// =====================================================================
		auto vertexInput = VulkanVertexInput::Build(spec.vertexLayout, 0);

		VkPipelineVertexInputStateCreateInfo viState{};
		viState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		viState.vertexBindingDescriptionCount = (uint32_t)vertexInput.bindings.size();
		viState.pVertexBindingDescriptions = vertexInput.bindings.data();
		viState.vertexAttributeDescriptionCount = (uint32_t)vertexInput.attributes.size();
		viState.pVertexAttributeDescriptions = vertexInput.attributes.data();

		// =====================================================================
		// 4. 고정 상태
		// =====================================================================
		VkPipelineInputAssemblyStateCreateInfo ia{};
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		ia.primitiveRestartEnable = VK_FALSE;

		// 뷰포트/시저는 동적 — 패널 크기가 바뀔 때마다 파이프라인을 다시 만들지 않기 위해서.
		// 실제 값은 VulkanRenderTarget::beginPass()가 vkCmdSetViewport/Scissor로 넣는다.
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
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;   // RenderTargetSpec.samples=1과 일치해야 함
		ms.sampleShadingEnable = VK_FALSE;

		// 깊이 clear가 1.0이고 GLM_FORCE_DEPTH_ZERO_TO_ONE(0~1)이므로 LESS가 맞다
		VkPipelineDepthStencilStateCreateInfo ds{};
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.depthTestEnable = spec.depthTest ? VK_TRUE : VK_FALSE;
		ds.depthWriteEnable = spec.depthWrite ? VK_TRUE : VK_FALSE;
		ds.depthCompareOp = VK_COMPARE_OP_LESS;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.stencilTestEnable = VK_FALSE;

		// 어태치먼트 개수는 렌더패스 subpass의 colorAttachmentCount와 반드시 같아야 한다.
		// ⚠️ blendEnable은 전부 FALSE여야 한다 — id 버퍼(R32_SINT)처럼 정수 포맷은
		//    블렌딩 자체가 금지다. 색 슬롯에 블렌딩을 켤 땐 슬롯별로 갈라야 한다.
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
		// 5. 파이프라인 생성
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

		// 모듈은 여기까지만 필요하다 — 파이프라인이 코드를 이미 흡수했다
		vkDestroyShaderModule(device, vertModule, nullptr);
		vkDestroyShaderModule(device, fragModule, nullptr);

		if (result != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("GraphicsPipeline createion failed! (VkResult {0})", (int)result);
			return;
		}

		APP_WMS_CORE_INFO("Pipeline build finished — attr {0}, stride {1}, push {2}B",
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

		// 이후 pushObjectMatrices가 이 파이프라인의 레이아웃을 찾아 쓸 수 있게 등록.
		// 추상 API로 VkPipelineLayout을 넘길 수 없어서 생긴 우회로다.
		static_cast<VulkanRendererAPI*>(&RendererCommand::getAPI())->setCurrentPipeline(this);
	}

	void VulkanPipeline::pushConstants(VkCommandBuffer cmd, const void* data, uint32_t size) const {
		if (m_PushConstantSize == 0) return;
		// 레이아웃에 선언한 범위를 넘기면 validation 에러 → 선언 크기로 자른다.
		// stageFlags도 선언과 정확히 같아야 하므로 스펙에서 받은 값을 그대로 쓴다.
		vkCmdPushConstants(cmd, m_Layout, m_PushConstantStages, 0,
			(size < m_PushConstantSize) ? size : m_PushConstantSize, data);
	}
}

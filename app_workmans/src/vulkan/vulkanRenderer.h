#pragma once
#include "core/core.h"
#include "renderer/rendererAPI.h"
#include <vulkan/vulkan.h>

namespace AppWMs {

	class VulkanPipeline;

	class VulkanRendererAPI : public RendererAPI {
	public:
		// for sub classes
		virtual void init() override;
		virtual bool beginFrame() override;
		virtual void endFrame() override;
		virtual void set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
		virtual void setClearColor(const glm::vec4& color) override;
		virtual void clear() override;
		virtual void drawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void drawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0) override;
		virtual void setLineWidth(float width) override;
		virtual void pushObjectMatrices(const glm::mat4& mvp, const glm::mat4& model,
			int32_t entityID) override;
		virtual void waitIdle() override;
		virtual void shutdown() override;

		//A push constant needs a VkPipelineLayout, but the abstract API cannot
		//carry Vulkan types. So VulkanPipeline::bind() registers itself here and
		//pushObjectMatrices borrows that layout.
		void setCurrentPipeline(const VulkanPipeline* pipeline) { m_CurrentPipeline = pipeline; }

		VkRenderPass getRenderPass() const { return m_RenderPass; }
		VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }

		//Framebuffers (paired with the RenderPass, so the same class owns them)
		//On resize, call this *after* the swapchain has been recreated
		void recreateFramebuffers();
		VkFramebuffer getFramebuffer(uint32_t index) const { return m_vkFramebuffers[index]; }
		uint32_t      getFramebufferCount() const { return (uint32_t)m_vkFramebuffers.size(); }
		const glm::vec4& getClearColor() const { return m_ClearColor; }
	private:
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkDescriptorPool     m_DescriptorPool = VK_NULL_HANDLE;
		//Mind the name -- these are Vulkan VkFramebuffer handles, not our FrameBuffer class.
		//Everything here is for the swapchain (screen output). The offscreen target for the
		//viewport panel is owned separately by VulkanRenderTarget.
		std::vector<VkFramebuffer> m_vkFramebuffers;
		glm::vec4 m_ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		const VulkanPipeline* m_CurrentPipeline = nullptr;   // not owned (borrowed reference)
	};

}

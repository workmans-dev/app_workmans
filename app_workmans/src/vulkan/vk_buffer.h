#pragma once
#include "renderer/buffer.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace AppWMs {

	class VulkanVertexBuffer : public VertexBuffer {
	public:
		// 정적: 데이터 즉시 업로드 (device-local, staging 경유)
		VulkanVertexBuffer(float* vertices, uint32_t size);
		// 동적: 빈 버퍼 (host-visible, 매 프레임 setData)
		VulkanVertexBuffer(uint32_t size);
		virtual ~VulkanVertexBuffer();
		// is this being used???---------------
		virtual void bind() const override {}    // Vulkan은 draw 기록 시 바인딩 (여기선 no-op)
		virtual void unbind() const override {}
		// ----------------------------------------
		virtual const BufferLayout& getLayout() const override { return m_Layout; }
		virtual void setLayout(const BufferLayout& layout) override { m_Layout = layout; }
		virtual void setData(const void* VB, uint32_t size) override;

		VkBuffer getHandle() const { return m_Buffer; }

	private:
		VkBuffer      m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Alloc = nullptr;
		void* m_Mapped = nullptr;   // 동적일 때만 유효
		uint32_t      m_Size = 0;
		bool          m_Dynamic = false;
		BufferLayout  m_Layout;
	};

	class VulkanIndexBuffer : public IndexBuffer {
	public:
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		virtual void bind() const override {}
		virtual void unbind() const override {}
		virtual uint32_t getCount() const override { return m_Count; }

		VkBuffer getHandle() const { return m_Buffer; }

	private:
		VkBuffer      m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Alloc = nullptr;
		uint32_t      m_Count = 0;
	};
}
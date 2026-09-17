#include "pch.h"
#include "vulkan/vk_buffer.h"
#include "vulkan/vulkanContext.h"
#include "vulkan/vk_transfer.h"

namespace AppWMs {

	// ─────────────────────────────────────────────
	// VulkanVertexBuffer
	// ─────────────────────────────────────────────

	// 정적: device-local + staging 업로드
	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
		: m_Size(size), m_Dynamic(false)
	{
		VmaAllocator allocator = VulkanContext::Get()->getAllocator();

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
			| VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI{};
		allocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;  // VRAM 우선

		if (vmaCreateBuffer(allocator, &bufferCI, &allocCI,
			&m_Buffer, &m_Alloc, nullptr) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("정적 VertexBuffer 생성 실패!");
			return;
		}

		// staging 경유 업로드 (정점 입력 단계에서 읽힘)
		VulkanTransfer::UploadBuffer(m_Buffer, vertices, size,
			VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	}

	// 동적: host-visible, 매핑 유지
	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
		: m_Size(size), m_Dynamic(true)
	{
		VmaAllocator allocator = VulkanContext::Get()->getAllocator();

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI{};
		allocCI.usage = VMA_MEMORY_USAGE_AUTO;
		allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
			| VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo outInfo{};
		if (vmaCreateBuffer(allocator, &bufferCI, &allocCI,
			&m_Buffer, &m_Alloc, &outInfo) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("동적 VertexBuffer 생성 실패!");
			return;
		}
		m_Mapped = outInfo.pMappedData;   // 매핑 포인터 보관
	}

	VulkanVertexBuffer::~VulkanVertexBuffer() {
		if (m_Buffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(VulkanContext::Get()->getAllocator(), m_Buffer, m_Alloc);
		}
	}

	void VulkanVertexBuffer::setData(const void* VB, uint32_t size) {
		if (!m_Dynamic) {
			APP_WMS_CORE_ERROR("정적 VertexBuffer에는 setData 불가!");
			return;
		}
		std::memcpy(m_Mapped, VB, (size_t)size);
	}

	// ─────────────────────────────────────────────
	// VulkanIndexBuffer  (정적 device-local)
	// ─────────────────────────────────────────────

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		VmaAllocator allocator = VulkanContext::Get()->getAllocator();
		VkDeviceSize size = (VkDeviceSize)count * sizeof(uint32_t);

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
			| VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI{};
		allocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		if (vmaCreateBuffer(allocator, &bufferCI, &allocCI,
			&m_Buffer, &m_Alloc, nullptr) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("IndexBuffer 생성 실패!");
			return;
		}

		VulkanTransfer::UploadBuffer(m_Buffer, indices, size,
			VK_ACCESS_INDEX_READ_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer() {
		if (m_Buffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(VulkanContext::Get()->getAllocator(), m_Buffer, m_Alloc);
		}
	}
}
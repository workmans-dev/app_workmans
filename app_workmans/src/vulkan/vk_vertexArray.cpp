#include "pch.h"
#include "vulkan/vk_vertexArray.h"
#include "vulkan/vk_buffer.h"

namespace AppWMs {

	// ---------------------------------------------
	// VulkanVertexInput -- BufferLayout -> Vulkan vertex input descriptors
	// ---------------------------------------------
	namespace VulkanVertexInput {

		VkFormat ToVkFormat(ShaderDataType type) {
			switch (type) {
			case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Int:    return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:   return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:   return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:   return VK_FORMAT_R32G32B32A32_SINT;
			case ShaderDataType::Bool:   return VK_FORMAT_R8_UINT;

			// Used as a vertex attribute, Mat3/Mat4 occupy 3-4 locations.
			// The current conversion rule is one attribute = one location, so it cannot express that.
			// Handle it on a separate path once instancing lands.
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
				APP_WMS_CORE_ASSERTS(false, "Mat3/Mat4 vertex attributes unsupported (they occupy multiple locations)");
				return VK_FORMAT_UNDEFINED;
			}
			APP_WMS_CORE_ASSERTS(false, "Unknown ShaderDataType!");
			return VK_FORMAT_UNDEFINED;
		}

		Description Build(const BufferLayout& layout, uint32_t binding) {
			Description desc;

			// stride is the value BufferLayout already accumulated in calculateOffsetAndStride
			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = binding;
			bindingDesc.stride = layout.getStride();
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			desc.bindings.push_back(bindingDesc);

			uint32_t location = 0;
			for (const auto& element : layout) {
				VkVertexInputAttributeDescription attr{};
				attr.location = location++;
				attr.binding = binding;
				attr.format = ToVkFormat(element.shaderDataType);
				attr.offset = element.offset;   // byte offset the layout computed
				desc.attributes.push_back(attr);
			}
			return desc;
		}
	}

	void VulkanVertexArray::addVertexBuffer(const Ref<VertexBuffer> vertexBuffer) {
		APP_WMS_CORE_ASSERTS(vertexBuffer->getLayout().getElements().size(),
			"VertexBuffer has no layout!")

			m_VertexBuffers.push_back(vertexBuffer);

		// Pull just the real VkBuffer handle out of the abstract pointer and put it in the cache
		auto vkVB = std::static_pointer_cast<VulkanVertexBuffer>(vertexBuffer);
		m_VkBuffers.push_back(vkVB->getHandle());
		m_Offsets.push_back(0);
	}

	void VulkanVertexArray::setIndexBuffer(const Ref<IndexBuffer> indexBuffer) {
		m_IndexBuffer = indexBuffer;
	}

	void VulkanVertexArray::bindToCommandBuffer(VkCommandBuffer cmd) const {
		if (!m_VkBuffers.empty()) {
			vkCmdBindVertexBuffers(cmd, 0, (uint32_t)m_VkBuffers.size(),
				m_VkBuffers.data(), m_Offsets.data());
		}
		if (m_IndexBuffer) {
			auto vkIB = std::static_pointer_cast<VulkanIndexBuffer>(m_IndexBuffer);
			// IndexBuffer::create takes uint32_t*, so UINT32 is fixed
			vkCmdBindIndexBuffer(cmd, vkIB->getHandle(), 0, VK_INDEX_TYPE_UINT32);
		}
	}
}

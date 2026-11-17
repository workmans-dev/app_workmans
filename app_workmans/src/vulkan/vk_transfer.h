#pragma once
#pragma once
#include <vulkan/vulkan.h>

namespace AppWMs {
	namespace VulkanTransfer {

		// One-shot command buffer (for immediate work such as transfers and layout transitions)
		VkCommandBuffer BeginSingleTime(VkDevice device, VkCommandPool pool);
		void EndSingleTime(VkDevice device, VkCommandPool pool, VkQueue queue, VkCommandBuffer cmd);

		// staging -> device-local upload
		// dstBuffer must have been created with VK_BUFFER_USAGE_TRANSFER_DST_BIT
		// With a dedicated transfer queue, this also handles the ownership transfer barriers (release/acquire)
		void UploadBuffer(VkBuffer dstBuffer, const void* data, VkDeviceSize size,
			VkAccessFlags dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	}
}

#include "pch.h"
#include "vulkan/vk_transfer.h"
#include "vulkan/vulkanContext.h"

namespace AppWMs {
	namespace VulkanTransfer {

		VkCommandBuffer BeginSingleTime(VkDevice device, VkCommandPool pool) {
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = pool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer cmd = VK_NULL_HANDLE;
			vkAllocateCommandBuffers(device, &allocInfo, &cmd);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(cmd, &beginInfo);
			return cmd;
		}

		void EndSingleTime(VkDevice device, VkCommandPool pool, VkQueue queue, VkCommandBuffer cmd) {
			vkEndCommandBuffer(cmd);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd;

			// Wait locally with a fence (vkQueueWaitIdle stalls the whole queue -> fatal for threaded uploads)
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			VkFence fence = VK_NULL_HANDLE;
			vkCreateFence(device, &fenceInfo, nullptr, &fence);

			vkQueueSubmit(queue, 1, &submitInfo, fence);
			vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

			vkDestroyFence(device, fence, nullptr);
			vkFreeCommandBuffers(device, pool, 1, &cmd);
		}

		void UploadBuffer(VkBuffer dstBuffer, const void* data, VkDeviceSize size,
			VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) {

			auto ctx = VulkanContext::Get();
			VkDevice     device = ctx->getDevice();
			VmaAllocator allocator = ctx->getAllocator();
			const auto& indices = ctx->getQueueIndices();

			const uint32_t transferFamily = indices.transferFamily.value();
			const uint32_t graphicsFamily = indices.graphicsFamily.value();
			const bool     dedicated = indices.hasDedicatedTransfer();

			// 1. Create the staging buffer (host-visible, auto-mapped)
			VkBufferCreateInfo stagingCI{};
			stagingCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			stagingCI.size = size;
			stagingCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo stagingAI{};
			stagingAI.usage = VMA_MEMORY_USAGE_AUTO;
			stagingAI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
				| VMA_ALLOCATION_CREATE_MAPPED_BIT;

			VkBuffer          staging = VK_NULL_HANDLE;
			VmaAllocation     stagingMem = nullptr;
			VmaAllocationInfo stagingOut{};
			if (vmaCreateBuffer(allocator, &stagingCI, &stagingAI,
				&staging, &stagingMem, &stagingOut) != VK_SUCCESS) {
				APP_WMS_CORE_ERROR("Failed to create staging buffer!");
				return;
			}

			memcpy(stagingOut.pMappedData, data, (size_t)size);
			vmaFlushAllocation(allocator, stagingMem, 0, size);

			// 2. Transfer queue: copy + (if dedicated) the release barrier
			VkCommandBuffer cmd = BeginSingleTime(device, ctx->getTransferCommandPool());

			VkBufferCopy copyRegion{};
			copyRegion.size = size;
			vkCmdCopyBuffer(cmd, staging, dstBuffer, 1, &copyRegion);

			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.buffer = dstBuffer;
			barrier.offset = 0;
			barrier.size = size;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			if (dedicated) {
				// release: dstAccessMask must be 0; ownership is handed over to graphics
				barrier.dstAccessMask = 0;
				barrier.srcQueueFamilyIndex = transferFamily;
				barrier.dstQueueFamilyIndex = graphicsFamily;
				vkCmdPipelineBarrier(cmd,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					0, 0, nullptr, 1, &barrier, 0, nullptr);
			}
			else {
				// Same family -> no ownership transfer, just a plain memory barrier
				barrier.dstAccessMask = dstAccessMask;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				vkCmdPipelineBarrier(cmd,
					VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask,
					0, 0, nullptr, 1, &barrier, 0, nullptr);
			}

			EndSingleTime(device, ctx->getTransferCommandPool(), ctx->getTransferQueue(), cmd);

			// 3. If it was a dedicated queue, graphics takes ownership with an acquire barrier
			if (dedicated) {
				VkCommandBuffer gcmd = BeginSingleTime(device, ctx->getGraphicsCommandPool());

				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = dstAccessMask;
				barrier.srcQueueFamilyIndex = transferFamily;
				barrier.dstQueueFamilyIndex = graphicsFamily;
				vkCmdPipelineBarrier(gcmd,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dstStageMask,
					0, 0, nullptr, 1, &barrier, 0, nullptr);

				EndSingleTime(device, ctx->getGraphicsCommandPool(), ctx->getGraphicsQueue(), gcmd);
			}

			// 4. Clean up staging
			vmaDestroyBuffer(allocator, staging, stagingMem);
		}
	}
}
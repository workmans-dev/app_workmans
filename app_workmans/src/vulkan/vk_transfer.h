#pragma once
#pragma once
#include <vulkan/vulkan.h>

namespace AppWMs {
    namespace VulkanTransfer {

        // 일회성 커맨드버퍼 (전송·레이아웃 전환 등 즉시 실행용)
        VkCommandBuffer BeginSingleTime(VkDevice device, VkCommandPool pool);
        void EndSingleTime(VkDevice device, VkCommandPool pool, VkQueue queue, VkCommandBuffer cmd);

        // staging → device-local 업로드
        // dstBuffer는 VK_BUFFER_USAGE_TRANSFER_DST_BIT로 생성되어 있어야 함
        // 전용 전송 큐인 경우 ownership transfer 배리어(release/acquire)까지 처리
        void UploadBuffer(VkBuffer dstBuffer, const void* data, VkDeviceSize size,
            VkAccessFlags dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT,
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
    }
}
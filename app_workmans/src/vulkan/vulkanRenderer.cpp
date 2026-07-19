#include "pch.h"
#include "vulkanRenderer.h"
#include "vulkan/vulkanContext.h" // 장치 정보 접근용

namespace AppWMs {

    void VulkanRendererAPI::init() {
        APP_WMS_CORE_INFO("VulkanRendererAPI: RenderPass & DescriptorPool !");

        VkDevice device = VulkanContext::Get()->getDevice();
        // 스왑체인 포맷은 vk_swapChain.cpp에서 ChooseSwapSurfaceFormat으로 고정해둔 VK_FORMAT_B8G8R8A8_SRGB를 씁니다.
        VkFormat swapChainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;

        // =========================================================================
        // 🎨 1. RenderPass (도화지 세팅) 조립
        // =========================================================================
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // 그리기 전에 화면 지움
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 그린 후 화면 보존
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 최종 화면 송출용 레이아웃

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("RenderPass 생성 실패!");
        }

        // =========================================================================
        // 🧺 2. DescriptorPool (물감 바구니) 조립
        // =========================================================================
        // ImGui 및 엔진 텍스처들이 넉넉하게 쓸 수 있도록 크기를 크게 잡아줍니다.
        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * 11; // 넉넉한 최대 세트 수
        pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_info.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(device, &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            APP_WMS_CORE_ERROR("DescriptorPool 생성 실패!");
        }

        APP_WMS_CORE_INFO("VulkanRendererAPI: RenderPass & DescriptorPool 생성 완료!");
    }

    // ... 나머지 빈 껍데기 함수들 구현부 생략 (clear, set_viewport 등) ...
}
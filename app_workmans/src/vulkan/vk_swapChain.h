#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "vulkan/vk_init.h"


struct GLFWwindow;

namespace AppWMs {
    namespace VulkanSwapChain {

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        struct SwapChainBundle {
            VkSwapchainKHR handle = VK_NULL_HANDLE;
            VkFormat       format = VK_FORMAT_UNDEFINED;
            VkExtent2D     extent = { 0, 0 };
            std::vector<VkImage>     images;
            std::vector<VkImageView> imageViews;   // 🟢 프레임버퍼 부착용 뷰
            // ⚠️ 깊이 없음 — 스왑체인 패스는 ImGui 전용(깊이 미사용).
            //    깊이는 오프스크린 프레임버퍼가 자기 크기로 따로 소유한다.
        };

        // 🟢 메인 가동 함수
        SwapChainBundle CreateSwapChain(VkPhysicalDevice physicalDevice, VkDevice device,
            VkSurfaceKHR surface, GLFWwindow* window, const VulkanInit::QueueFamilyIndices& indices);

        // 🟢 뷰 + 스왑체인 파괴 후 번들을 비운다. 재생성/종료 양쪽에서 쓴다.
        // ⚠️ 호출 전에 GPU가 이 리소스를 다 썼음이 보장돼야 한다 (vkDeviceWaitIdle).
        //    images는 드라이버 소유라 우리가 파괴하지 않는다 — 스왑체인과 함께 사라진다.
        void DestroySwapChain(VkDevice device, SwapChainBundle& bundle);
        // 🟢 포맷 조회 헬퍼 — 오프스크린 깊이 생성에서 재사용
        VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

        // 🟢 [보완] 컴파일러가 찾지 못했던 헬퍼 함수들의 정식 선언 등록
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        VkImageView CreateImageView(VkDevice device, VkImage image,
            VkFormat format, VkImageAspectFlags aspectFlags);
    }
}
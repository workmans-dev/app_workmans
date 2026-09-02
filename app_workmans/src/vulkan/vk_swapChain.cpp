#include "pch.h"
#include "vulkan/vk_swapChain.h"
#include <GLFW/glfw3.h>


namespace AppWMs {
    namespace VulkanSwapChain {

        // 1. 하드웨어 스펙 및 모니터 지원 능력 싹 긁어오기
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
            SwapChainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }
            return details;
        }

        // 2. 최적의 색상 선택 함수 내부 알맹이
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }
            return availableFormats[0];
        }

        // 3. 최적의 동기화 방식(삼중 버퍼링 등) 선택 함수 내부 알맹이
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR; // 수직동기화 기본값 보장
        }

        // 4. 해상도 크기 결정 함수 내부 알맹이
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
            if (capabilities.currentExtent.width != UINT32_MAX) {
                return capabilities.currentExtent;
            }
            else {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
                actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
                return actualExtent;
            }
        }

        // 4-1. 범용 이미지뷰 생성기
        VkImageView CreateImageView(VkDevice device, VkImage image,
            VkFormat format, VkImageAspectFlags aspectFlags) {

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;

            viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView view = VK_NULL_HANDLE;
            if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("ImageView 생성 실패!");
                return VK_NULL_HANDLE;
            }
            return view;
        }

        // 4-2. 하드웨어 지원 포맷 탐색
        VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling, VkFormatFeatureFlags features) {

            for (VkFormat format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR &&
                    (props.linearTilingFeatures & features) == features) {
                    return format;
                }
                if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                    (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }
            APP_WMS_CORE_ERROR("지원되는 포맷을 찾지 못함!");
            return VK_FORMAT_UNDEFINED;
        }

        // 4-3. 깊이 포맷 우선순위: 정밀도 높은 순, 스텐실 없는 것 우선
        VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice) {
            return FindSupportedFormat(physicalDevice,
                { VK_FORMAT_D32_SFLOAT,
                  VK_FORMAT_D32_SFLOAT_S8_UINT,
                  VK_FORMAT_D24_UNORM_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }

        // 5. 메인 스왑체인 최종 조립 함수
        SwapChainBundle CreateSwapChain(VkPhysicalDevice physicalDevice, VkDevice device,
            VkSurfaceKHR surface, GLFWwindow* window, const VulkanInit::QueueFamilyIndices& indices) {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);

            VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
            VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            // get all information from vk_init and set -------------------
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices = nullptr;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;
            // ------------------------------------------------------------------------

            SwapChainBundle bundle;
            if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &bundle.handle) != VK_SUCCESS) {
                return {};
            }
            bundle.format = surfaceFormat.format;
            bundle.extent = extent;

            uint32_t actualImageCount = 0;
            vkGetSwapchainImagesKHR(device, bundle.handle, &actualImageCount, nullptr);
            bundle.images.resize(actualImageCount);
            vkGetSwapchainImagesKHR(device, bundle.handle, &actualImageCount, bundle.images.data());

            // 🟢 각 스왑체인 이미지를 컬러 뷰로 감싸기
            bundle.imageViews.resize(actualImageCount);
            for (uint32_t i = 0; i < actualImageCount; i++) {
                bundle.imageViews[i] = CreateImageView(device, bundle.images[i],
                    bundle.format, VK_IMAGE_ASPECT_COLOR_BIT);
            }

            APP_WMS_CORE_INFO("SwapChain: {0} images/views created ({1}x{2})",
                actualImageCount, bundle.extent.width, bundle.extent.height);

            return bundle;
        }

        // 6. 파괴 — 재생성과 종료가 공유한다
        void DestroySwapChain(VkDevice device, SwapChainBundle& bundle) {
            for (VkImageView view : bundle.imageViews) {
                if (view != VK_NULL_HANDLE) vkDestroyImageView(device, view, nullptr);
            }
            bundle.imageViews.clear();

            // images는 vkGetSwapchainImagesKHR로 "받아온" 드라이버 소유 핸들이라
            // 개별 파괴하지 않는다. 스왑체인이 사라지면 같이 무효가 된다.
            bundle.images.clear();

            if (bundle.handle != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(device, bundle.handle, nullptr);
                bundle.handle = VK_NULL_HANDLE;
            }
            bundle.extent = { 0, 0 };
        }
    }
}
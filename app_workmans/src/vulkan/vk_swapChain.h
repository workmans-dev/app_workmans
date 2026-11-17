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
			std::vector<VkImageView> imageViews;   // views to attach to the framebuffer
			// WARNING: no depth -- the swapchain pass is ImGui only (depth unused).
			//          Depth is owned separately by the offscreen framebuffer, at its own size.
		};

		// Main entry point
		SwapChainBundle CreateSwapChain(VkPhysicalDevice physicalDevice, VkDevice device,
			VkSurfaceKHR surface, GLFWwindow* window, const VulkanInit::QueueFamilyIndices& indices);

		// Destroys the views and the swapchain, then clears the bundle. Used both on recreate and on shutdown.
		// WARNING: the GPU must be guaranteed done with these resources before the call (vkDeviceWaitIdle).
		//          images are driver-owned, so we do not destroy them -- they go away with the swapchain.
		void DestroySwapChain(VkDevice device, SwapChainBundle& bundle);
		// Format query helper -- reused when creating the offscreen depth
		VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

		// Proper declarations for the helpers the compiler could not find
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

		VkImageView CreateImageView(VkDevice device, VkImage image,
			VkFormat format, VkImageAspectFlags aspectFlags);
	}
}

#pragma once
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace AppWMs {
	namespace VulkanInit {

		// Bucket of tunable settings that a UI client (ImGui etc.) or a config system will drive later
		struct EngineGraphicsSettings {
			bool enableGeometryShader = false;
			bool enableWideLines = false;
			bool enableRayTracing = false; // for future expansion
			// Keep adding UI-driven options here: shadow quality, anti-aliasing, and so on.
		};

		// Container for the graphics / present / transfer queue family indices
		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			std::optional<uint32_t> transferFamily; // dedicated DMA transfer queue (staging->device uploads)

			// Helper that checks whether all three indices were found
			bool isComplete() const {
				return graphicsFamily.has_value()
					&& presentFamily.has_value()
					&& transferFamily.has_value();
			}

			// Whether a dedicated transfer queue (a family different from graphics) was actually found.
			// False when it fell back to the same family -> the ownership transfer barriers can be skipped.
			bool hasDedicatedTransfer() const {
				return transferFamily.has_value()
					&& transferFamily != graphicsFamily;
			}
		};

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkInstance CreateInstance();
		VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window);
		VkPhysicalDevice PickPhysicalDevice(VkInstance instance);

		// Upgraded to take the settings struct as an argument
		VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice,
			const QueueFamilyIndices& indices, const EngineGraphicsSettings& settings);

		VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueFamilyIndex,
			VkCommandPoolCreateFlags flags);
	}
}


#include "pch.h"
// should be included first !! because of "glfwCreateWindowSurface"
#include "vk_init.h"
#include <GLFW/glfw3.h>



namespace AppWMs {
    namespace VulkanInit {
        VkInstance CreateInstance() {
            // 1. Application info (optional, but we fill it in as a matter of course)
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "WORKMANs";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "WMs Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3; // we target Vulkan 1.3

            // 2. Prepare the instance create-info container
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;



            // 3. Collect the extensions GLFW requires (VK_KHR_surface, VK_KHR_win32_surface)
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            // Enable the validation layer in debug builds only
            std::vector<const char*> layers;
#ifdef APP_WMS_DEBUG
            {
                const char* kValidation = "VK_LAYER_KHRONOS_validation";
                uint32_t layerCount = 0;
                vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
                std::vector<VkLayerProperties> available(layerCount);
                vkEnumerateInstanceLayerProperties(&layerCount, available.data());

                bool found = false;
                for (const auto& p : available) {
                    if (strcmp(p.layerName, kValidation) == 0) { found = true; break; }
                }
                if (found) {
                    layers.push_back(kValidation);
                    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    APP_WMS_CORE_INFO("Validation layer enabled");
                }
                else {
                    APP_WMS_CORE_WARN("VK_LAYER_KHRONOS_validation not found - check the SDK layer install");
                }
            }
#endif
            createInfo.enabledExtensionCount = (uint32_t)extensions.size();
            createInfo.ppEnabledExtensionNames = extensions.data();
            createInfo.enabledLayerCount = (uint32_t)layers.size();
            createInfo.ppEnabledLayerNames = layers.data();
            createInfo.pNext = nullptr;



            // 4. Create the instance and secure the hardware context
            VkInstance instance;
            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                // Report the error through the project log macros or std::cerr.
                APP_WMS_CORE_ERROR("failed to create Vulkan instance!");
                return VK_NULL_HANDLE;
            }
            return instance;
        }

        VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window) {
            VkSurfaceKHR surface;
            // GLFW handles all the per-OS surface creation complexity for us.
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                // Report the error through the project log system.
                APP_WMS_CORE_ERROR("failed to create window surface!");
                return VK_NULL_HANDLE;
            }
            return surface;
        }

        VkPhysicalDevice PickPhysicalDevice(VkInstance instance) 
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
            int maxScore = -1;

            for (const auto& device : devices) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(device, &properties);

                int currentScore = 0;
                // Scoring: a discrete GPU (RTX and the like) gets an overwhelming score
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    currentScore += 10000;
                }
                // Integrated graphics (Intel Iris Xe and the like) only gets the base score
                else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
                    currentScore += 1000;
                }

                // Pick the highest-scoring (fastest) GPU
                if (currentScore > maxScore) {
                    maxScore = currentScore;
                    bestDevice = device;
                }
                APP_WMS_CORE_INFO("Found GPU: {0} (Score: {1})", properties.deviceName, currentScore);
            }
            return bestDevice;
        }
  
        // Internal-only helper that automates the queue family search
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
            QueueFamilyIndices indices;

            // 1. Query how many physical queue families this GPU has
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            // 2. Look for the graphics / present families
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                const auto& flags = queueFamilies[i].queueFlags;
                // Can this family do graphics work?
                if (flags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }
                // Can this family present to the surface we created?
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (presentSupport) {
                    indices.presentFamily = i;
                }
            }

            // 3. Prefer a dedicated transfer family:
            //    look for a pure DMA family that has the TRANSFER bit but not the GRAPHICS bit.
            //    (every graphics queue can implicitly transfer, but a separate DMA engine needs its own family)
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                const auto& flags = queueFamilies[i].queueFlags;
                if ((flags & VK_QUEUE_TRANSFER_BIT) &&
                    !(flags & VK_QUEUE_GRAPHICS_BIT)) {
                    indices.transferFamily = i;
                    break; // dedicated family found -> settled
                }
            }

            // 4. With no dedicated transfer family, fall back to graphics (same family -> barriers skipped later)
            if (!indices.transferFamily.has_value()) {
                indices.transferFamily = indices.graphicsFamily;
            }

            return indices;
        }



        VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, 
            const QueueFamilyIndices& indices, const EngineGraphicsSettings& settings) {

            // Build the queue create-info array to attach to the device create info
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

            std::set<uint32_t> uniqueQueueFamilies = {
                indices.graphicsFamily.value(),
                indices.presentFamily.value(),
                indices.transferFamily.value()
            };

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            // 2. Query hardware features and map the user-tunable settings (unchanged)
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

            VkPhysicalDeviceFeatures deviceFeatures{};
            if (supportedFeatures.geometryShader && settings.enableGeometryShader) deviceFeatures.geometryShader = VK_TRUE;
            if (supportedFeatures.wideLines && settings.enableWideLines) deviceFeatures.wideLines = VK_TRUE;

            // 3. Assemble the final create info
            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data(); // hand over the assembled queue set
            createInfo.pEnabledFeatures = &deviceFeatures;

            // IMPORTANT: to use a swapchain, the logical device must declare the presentation extension at creation time.
            const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME // the macro for the "VK_KHR_swapchain" string
            };
            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            VkDevice device;
            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("failed to create logical device!");
                return VK_NULL_HANDLE;
            }

            return device;
        }

        VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueFamilyIndex,
            VkCommandPoolCreateFlags flags) {

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndex;
            poolInfo.flags = flags;

            VkCommandPool pool;
            if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
                APP_WMS_CORE_ERROR("failed to create command pool! (family {0})", queueFamilyIndex);
                return VK_NULL_HANDLE;
            }
            return pool;
        }


    }
}
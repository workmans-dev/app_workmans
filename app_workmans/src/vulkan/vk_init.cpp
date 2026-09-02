
#include "pch.h"
// should be included first !! because of "glfwCreateWindowSurface"
#include "vk_init.h"
#include <GLFW/glfw3.h>



namespace AppWMs {
    namespace VulkanInit {
        VkInstance CreateInstance() {
            // 1. 애플리케이션 정보 설정 (선택 사항이지만 기본적으로 채우는 서류)
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "WORKMANs";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "WMs Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3; // 벌칸 1.3 버전 사용 사양

            // 2. 최종 인스턴스 생성 정보 컨테이너 준비
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;



            // 3. GLFW 필수 확장(VK_KHR_surface, VK_KHR_win32_surface) 수집
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            // 🟢 디버그 빌드에서만 validation layer 활성화
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
                    APP_WMS_CORE_WARN("VK_LAYER_KHRONOS_validation not found - SDK layer install 확인");
                }
            }
#endif
            createInfo.enabledExtensionCount = (uint32_t)extensions.size();
            createInfo.ppEnabledExtensionNames = extensions.data();
            createInfo.enabledLayerCount = (uint32_t)layers.size();
            createInfo.ppEnabledLayerNames = layers.data();
            createInfo.pNext = nullptr;



            // 4. 실제 인스턴스 생성 및 하드웨어 컨텍스트 확보
            VkInstance instance;
            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                // 형님 프로젝트의 로그 매크로나 std::cerr을 사용해 에러를 뿜게 합니다.
                APP_WMS_CORE_ERROR("failed to create Vulkan instance!");
                return VK_NULL_HANDLE;
            }
            return instance;
        }

        VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window) {
            VkSurfaceKHR surface;
            // GLFW가 OS별 서피스 생성 복잡도를 완전히 대행해 줍니다.
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                // 형님 프로젝트의 로그 시스템에 맞춰 에러를 뿌려줍니다.
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
                // 🟢 가점제 방식: 하드웨어 타입이 외장 그래픽(RTX 등)이면 압도적인 점수를 부여!
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    currentScore += 10000;
                }
                // 내장 그래픽(Intel Iris Xe 등)은 기본 점수만 부여
                else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
                    currentScore += 1000;
                }

                // 가장 점수가 높은 (가장 성능이 좋은) GPU를 최종 선택
                if (currentScore > maxScore) {
                    maxScore = currentScore;
                    bestDevice = device;
                }
                APP_WMS_CORE_INFO("Found GPU: {0} (Score: {1})", properties.deviceName, currentScore);
            }
            return bestDevice;
        }
  
        // 🟢 내부적으로만 쓰일 통로 검색 자동화 헬퍼 함수
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
            QueueFamilyIndices indices;

            // 1. 이 GPU가 가진 물리적인 통로(큐 패밀리) 개수 조회
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            // 2. graphics / present 통로 탐색
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                const auto& flags = queueFamilies[i].queueFlags;
                // 그래픽 연산이 가능한 통로인가?
                if (flags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }
                // ⭐️ 이 통로가 우리가 만든 도화지(surface)에 그림을 던질 수 있는가?
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (presentSupport) {
                    indices.presentFamily = i;
                }
            }

            // 3. ⭐️ 전용 transfer 통로 우선 탐색:
            //    "TRANSFER 비트는 있으면서 GRAPHICS 비트는 없는" 순수 DMA 전용 패밀리를 찾는다.
            //    (모든 graphics 큐는 암묵적으로 transfer 가능하지만, 별도 DMA 엔진을 쓰려면 분리 패밀리가 필요)
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                const auto& flags = queueFamilies[i].queueFlags;
                if ((flags & VK_QUEUE_TRANSFER_BIT) &&
                    !(flags & VK_QUEUE_GRAPHICS_BIT)) {
                    indices.transferFamily = i;
                    break; // 전용 통로 발견 → 확정
                }
            }

            // 4. 전용 전송 통로가 없으면 graphics 통로로 폴백 (같은 패밀리 → 나중에 배리어 생략됨)
            if (!indices.transferFamily.has_value()) {
                indices.transferFamily = indices.graphicsFamily;
            }

            return indices;
        }



        VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, 
            const QueueFamilyIndices& indices, const EngineGraphicsSettings& settings) {

            // 가동 서류에 첨부할 큐 생성 정보 배열 준비
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

            // 2. 하드웨어 스펙 조회 및 유저 가변 세팅 매핑 (기존과 동일)
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

            VkPhysicalDeviceFeatures deviceFeatures{};
            if (supportedFeatures.geometryShader && settings.enableGeometryShader) deviceFeatures.geometryShader = VK_TRUE;
            if (supportedFeatures.wideLines && settings.enableWideLines) deviceFeatures.wideLines = VK_TRUE;

            // 3. 최종 주문서 조립
            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data(); // 조립된 일꾼 세트들 전달
            createInfo.pEnabledFeatures = &deviceFeatures;

            // ⭐️ 중요: 스왑체인을 쓰기 위해서는 논리 장치를 켤 때 "나 화면 출력 확장 기능 쓸 거임"이라고 명시해야 합니다.
            const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME // "VK_KHR_swapchain" 매크로 문자열입니다.
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
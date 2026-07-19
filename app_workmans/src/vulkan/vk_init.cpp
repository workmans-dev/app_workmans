
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

            // 3. 핵심: GLFW가 화면을 띄우기 위해 필요한 필수 확장 기능(Extension) 목록 가져오기
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            // 3. to get VK_KHR_surface, VK_KHR_win32_surface
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            // 받아온 GLFW 확장 기능 정보들을 인스턴스 설정 컨테이너에 담아줍니다.
            createInfo.enabledExtensionCount = glfwExtensionCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;

            // 디버깅용 유효성 검사 레이어(Validation Layer)는 일단 지금은 0으로 비워두고 기본 구동에 집중합니다.
            createInfo.enabledLayerCount = 0;
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

            // 2. 루프를 돌며 내 조건에 맞는 통로 번호 싹 긁어오기
            for (int i = 0; i < queueFamilyCount; i++) {
                // 그래픽 연산이 가능한 통로인가?
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                // ⭐️ 핵심: 이 통로가 우리가 만든 도화지(surface)에 그림을 던질 수 있는가?
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                // 두 개 다 찾았으면 굳이 더 돌 필요 없이 조기 탈출!
                if (indices.isComplete()) {
                    break;
                }
            }
            return indices;
        }
  




        VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, 
            VkSurfaceKHR surface, const EngineGraphicsSettings& settings) {

            // 1. 자동화 로직으로 두 통로의 인덱스를 완벽하게 확보합니다.
            QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

            // 가동 서류에 첨부할 큐 생성 정보 배열 준비
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

            // 두 통로 번호가 같을 수도, 다를 수도 있으므로 std::set을 이용해 중복 번호를 걸러내어 컨테이너화합니다.
            std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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
    }
}
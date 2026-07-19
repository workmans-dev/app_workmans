#pragma once
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace AppWMs {
    namespace VulkanInit {

        // 🟢 나중에 UI 클라이언트(ImGui 등)나 환경설정 시스템이 직접 제어할 가변 변수 통
        struct EngineGraphicsSettings {
            bool enableGeometryShader = false;
            bool enableWideLines = false;
            bool enableRayTracing = false; // 향후 확장용
            // 여기에 그림자 품질, 안티앨리어싱 등 UI와 연동할 옵션들을 계속 추가하면 됩니다.
        };

        // 🟢 그래픽스 통로와 프레젠트(화면출력) 통로 번호를 담을 컨테이너
        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            // 두 통로의 번호를 성공적으로 다 찾았는지 확인하는 헬퍼 함수
            bool isComplete() const {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        VkInstance CreateInstance();
        VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window);
        VkPhysicalDevice PickPhysicalDevice(VkInstance instance);

        // 🟢 가변 설정 구조체를 인자로 받도록 업그레이드!
        VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface, const EngineGraphicsSettings& settings);
    }
}
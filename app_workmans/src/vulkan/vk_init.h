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

		// 🟢 그래픽스 / 프레젠트(화면출력) / 트랜스퍼(전송) 통로 번호를 담을 컨테이너
		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			std::optional<uint32_t> transferFamily; // 🟢 전용 DMA 전송 통로 (staging→device 업로드용)

			// 세 통로 번호를 성공적으로 다 찾았는지 확인하는 헬퍼 함수
			bool isComplete() const {
				return graphicsFamily.has_value()
					&& presentFamily.has_value()
					&& transferFamily.has_value();
			}

			// 🟢 전용 전송 큐(graphics와 다른 패밀리)를 실제로 찾았는지 여부.
			// 같은 패밀리로 폴백된 경우 false → ownership transfer 배리어 생략 가능.
			bool hasDedicatedTransfer() const {
				return transferFamily.has_value()
					&& transferFamily != graphicsFamily;
			}
		};

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkInstance CreateInstance();
		VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window);
		VkPhysicalDevice PickPhysicalDevice(VkInstance instance);

		// 🟢 가변 설정 구조체를 인자로 받도록 업그레이드!
		VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice,
			const QueueFamilyIndices& indices, const EngineGraphicsSettings& settings);

		VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueFamilyIndex,
			VkCommandPoolCreateFlags flags);
	}
}
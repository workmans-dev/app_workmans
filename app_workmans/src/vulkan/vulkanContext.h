#pragma once
#include "renderer/graphicContext.h"
#include <vulkan/vulkan.h> 

struct GLFWwindow;

namespace AppWMs {

	class VulkanContext : public GraphicContext {
	public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext() = default;

		virtual void init() override;
		virtual void swapBuffers() override;

		static VulkanContext* Get();

		// 🟢 하위 렌더러 구현체(VulkanRendererAPI)가 상위 레이어를 역참조하지 않고
		// 이 컨텍스트에서 하드웨어 정보를 깨끗하게 바인딩해 갈 수 있도록 게터 제공
		VkInstance getInstance() const { return m_Instance; }
		VkSurfaceKHR getSurface() const { return m_Surface; }
		VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice getDevice() const { return m_LogicalDevice; }
		VkSwapchainKHR getSwapChain() const { return m_SwapChain; }

	private:
		GLFWwindow* m_WindowHandle;
		static VulkanContext* s_VulkanContext;
		// 🟢 Win_Window에서 싹 다 빼내어 이 컨텍스트 내부로 완벽히 격리시킨 벌칸 코어 객체들
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
	};

}
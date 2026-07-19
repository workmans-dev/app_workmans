#include "pch.h"
#include "vulkanContext.h"
#include "vulkan/vk_init.h"
#include "vulkan/vk_swapChain.h"

namespace AppWMs {
	static VulkanContext* s_VulkanContext = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		
		APP_WMS_CORE_ASSERTS(windowHandle, "Window handle is null!")
	}

	void VulkanContext::init()
	{
		s_VulkanContext = this;
		APP_WMS_CORE_INFO("VulkanContext started");

		// 1. 벌칸 본사 핸들(Instance) 생성
		m_Instance = VulkanInit::CreateInstance();

		// 2. OS 창틀과 벌칸을 연결할 도화지(Surface) 생성
		m_Surface = VulkanInit::CreateSurface(m_Instance, m_WindowHandle);

		// 3. 컴퓨터에 꽂힌 진짜 글카(PhysicalDevice) 선택
		m_PhysicalDevice = VulkanInit::PickPhysicalDevice(m_Instance);

		// 4. 가변 그래픽 세팅 주입 및 논리 장치(Device) 생성
		VulkanInit::EngineGraphicsSettings settings;
		m_LogicalDevice = VulkanInit::CreateLogicalDevice(m_PhysicalDevice, m_Surface, settings);

		// 5. 모니터 스펙에 맞춘 메인 스왑체인(SwapChain) 최종 조립
		m_SwapChain = VulkanSwapChain::CreateSwapChain(m_PhysicalDevice, m_LogicalDevice, m_Surface, m_WindowHandle);

		APP_WMS_CORE_INFO("VulkanContext 인프라 완공 완료!");
	}

	void VulkanContext::swapBuffers()
	{
		// 🟢 바로 여기가 OpenGL의 glfwSwapBuffers()에 상응하는 벌칸의 최종 결산 지점입니다!
		// 매 프레임 다 조립된 명령어 장부(Command Buffer)를 GPU에 결재 올리고(vkQueueSubmit)
		// 완공된 그림을 모니터로 송출하는(vkQueuePresentKHR) 복잡한 신호등 동기화 처리가
		// 엔진 코어를 오염시키지 않고 오직 이 안에서 깔끔하게 은닉되어 가동됩니다.

		// [추후 구현될 동기화 및 Present 알맹이 예시]
		// vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
		// vkQueuePresentKHR(presentQueue, &presentInfo);
	}

	VulkanContext* VulkanContext::Get()
	{
		return s_VulkanContext;
	}

}
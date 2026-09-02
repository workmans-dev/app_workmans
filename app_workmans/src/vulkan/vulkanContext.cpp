#include "pch.h"
#include "vulkanContext.h"
#include <GLFW/glfw3.h>   // 최소화 판정용 프레임버퍼 크기 조회

namespace AppWMs {
	VulkanContext* VulkanContext::s_VulkanContext = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		
		APP_WMS_CORE_ASSERTS(windowHandle, "Window handle is null!")
	}

	void VulkanContext::init()
	{
		s_VulkanContext = this;
		APP_WMS_CORE_INFO("VulkanContext started");

		//  벌칸 본사 핸들(Instance) 생성
		m_Instance = VulkanInit::CreateInstance();

		//  OS 창틀과 벌칸을 연결할 도화지(Surface) 생성
		m_Surface = VulkanInit::CreateSurface(m_Instance, m_WindowHandle);

		//  컴퓨터에 꽂힌 진짜 글카(PhysicalDevice) 선택
		m_PhysicalDevice = VulkanInit::PickPhysicalDevice(m_Instance);

		// vk_init
		m_QueueIndices = VulkanInit::FindQueueFamilies(m_PhysicalDevice, m_Surface);

		//  가변 그래픽 세팅 주입 및 논리 장치(Device) 생성
		VulkanInit::EngineGraphicsSettings settings;
		m_LogicalDevice = VulkanInit::CreateLogicalDevice(m_PhysicalDevice, m_QueueIndices, settings);
		//  논리 장치에서 실제 큐 핸들 3종 인출
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.presentFamily.value(), 0, &m_PresentQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.transferFamily.value(), 0, &m_TransferQueue);

		APP_WMS_CORE_INFO("Queue family - graphics:{0}, present:{1}, transfer:{2} (dedicated transfer: {3})",
			m_QueueIndices.graphicsFamily.value(),
			m_QueueIndices.presentFamily.value(),
			m_QueueIndices.transferFamily.value(),
			m_QueueIndices.hasDedicatedTransfer());

		//  VMA 할당자 생성 (Instance/PhysicalDevice/LogicalDevice 모두 확보된 직후)
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		allocatorInfo.instance = m_Instance;
		allocatorInfo.physicalDevice = m_PhysicalDevice;
		allocatorInfo.device = m_LogicalDevice;
		if (vmaCreateAllocator(&allocatorInfo, &m_Allocator) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("Failed VMA Allocator !");
		}
		//  커맨드풀 2종 생성
		m_GraphicsCommandPool = VulkanInit::CreateCommandPool(m_LogicalDevice,
			m_QueueIndices.graphicsFamily.value(),VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		m_TransferCommandPool = VulkanInit::CreateCommandPool(m_LogicalDevice,
			m_QueueIndices.transferFamily.value(),VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

		// SwapChain
		m_SwapChainBundle = VulkanSwapChain::CreateSwapChain(m_PhysicalDevice, m_LogicalDevice,
			m_Surface, m_WindowHandle, m_QueueIndices);
		
		createFrameResources();
		APP_WMS_CORE_INFO("set VulkanContext successfully!");
	}

	void VulkanContext::createFrameResources()
	{
		// 1. 그래픽스 커맨드버퍼 (frame-in-flight당 1개)
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_GraphicsCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		if (vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("commandBuffer assigning failed!");
		}

		// 2. 프레임 슬롯에 묶인 동기화 객체 (개수 고정 → 재생성 대상 아님)
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semInfo{};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 첫 프레임 waitFence 즉시 통과용

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkCreateSemaphore(m_LogicalDevice, &semInfo, nullptr, &m_ImageAvailableSemaphores[i]);
			vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_InFlightFences[i]);
		}

		// 3. 이미지 개수에 묶인 것 — 스왑체인 재생성 때 개수가 바뀔 수 있어 따로 뺐다
		recreateImageSyncObjects();

		APP_WMS_CORE_INFO("Frame resources: cmdBuf {0}, imageAvailable/fence {0}, renderFinished {1}",
			MAX_FRAMES_IN_FLIGHT, m_SwapChainBundle.images.size());
	}

	void VulkanContext::recreateImageSyncObjects()
	{
		// renderFinished는 present가 기다리는 신호라 "이미지별"이어야 한다.
		// 프레임 슬롯별로 두면 이미지 개수와 어긋나 같은 세마포어를 두 present가 쓸 수 있다.
		for (VkSemaphore sem : m_RenderFinishedSemaphores) {
			if (sem != VK_NULL_HANDLE) vkDestroySemaphore(m_LogicalDevice, sem, nullptr);
		}

		const size_t imageCount = m_SwapChainBundle.images.size();
		m_RenderFinishedSemaphores.assign(imageCount, VK_NULL_HANDLE);
		// 펜스는 여기서 만들지 않는다 — 프레임 슬롯 펜스를 빌려 추적만 하는 배열이다
		m_ImagesInFlight.assign(imageCount, VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semInfo{};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (size_t i = 0; i < imageCount; i++) {
			vkCreateSemaphore(m_LogicalDevice, &semInfo, nullptr, &m_RenderFinishedSemaphores[i]);
		}
	}

	bool VulkanContext::recreateSwapChain()
	{
		// 최소화되면 프레임버퍼 크기가 0이 된다. extent 0으로는 스왑체인을 만들 수 없으므로
		// 복원될 때까지 낡음 표시를 유지한 채 물러난다 (App::run이 프레임을 스킵한다).
		int w = 0, h = 0;
		glfwGetFramebufferSize(m_WindowHandle, &w, &h);
		if (w == 0 || h == 0) {
			return false;
		}

		// 옛 스왑체인·뷰를 GPU가 아직 쓰고 있을 수 있다. ③ 결정과 같은 방식으로 막는다.
		vkDeviceWaitIdle(m_LogicalDevice);

		VulkanSwapChain::DestroySwapChain(m_LogicalDevice, m_SwapChainBundle);

		m_SwapChainBundle = VulkanSwapChain::CreateSwapChain(m_PhysicalDevice, m_LogicalDevice,
			m_Surface, m_WindowHandle, m_QueueIndices);

		if (m_SwapChainBundle.handle == VK_NULL_HANDLE) {
			APP_WMS_CORE_ERROR("SwapChain recreation failed!");
			return false;
		}

		recreateImageSyncObjects();

		// 옛 이미지 인덱스/슬롯은 의미가 없어졌다
		m_CurrentFrame = 0;
		m_CurrentImageIndex = 0;
		m_SwapChainDirty = false;
		return true;
	}

	bool VulkanContext::beginFrame()
	{
		// 1. 이 슬롯의 이전 GPU 작업이 끝날 때까지 CPU 대기 (커맨드버퍼 재사용 안전 보장)
		vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		// 2. 다음 스왑체인 이미지 확보 — 확보 완료를 imageAvailable 세마포어가 신호
		VkResult acquire = vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChainBundle.handle,
			UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImageIndex);

		if (acquire == VK_ERROR_OUT_OF_DATE_KHR) {
			// 여기서 바로 재생성하지 않는다 — 다음 프레임 경계에서 상위가 처리한다.
			m_SwapChainDirty = true;
			return false;
		}
		if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) {
			APP_WMS_CORE_ERROR("vkAcquireNextImageKHR 실패!");
			return false;
		}

		// 3. 확보한 이미지를 아직 다른 프레임이 GPU에서 쓰고 있으면 그 펜스도 대기
		if (m_ImagesInFlight[m_CurrentImageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_LogicalDevice, 1, &m_ImagesInFlight[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);
		}
		m_ImagesInFlight[m_CurrentImageIndex] = m_InFlightFences[m_CurrentFrame];

		// 4. 제출 확정 → 펜스 리셋 후 커맨드버퍼 개시
		vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

		VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("commandBuffer begin failed!");
			return false;
		}
		return true;
	}

	void VulkanContext::swapBuffers()
	{
		VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];

		if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("commandBuffer end failed!");
			return;
		}

		// 1. 제출: imageAvailable 대기(컬러 출력 단계) → 다 그리면 renderFinished 신호 + 펜스
		VkSemaphore          waitSem[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkSemaphore          signalSem[] = { m_RenderFinishedSemaphores[m_CurrentImageIndex] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSem;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSem;

		if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("vkQueueSubmit 실패!");
			return;
		}

		// 2. 화면 송출: renderFinished 대기 후 present
		VkSwapchainKHR swapChains[] = { m_SwapChainBundle.handle };
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSem;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_CurrentImageIndex;

		VkResult present = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
		if (present == VK_ERROR_OUT_OF_DATE_KHR || present == VK_SUBOPTIMAL_KHR) {
			// 이번 프레임은 이미 제출됐다. 재생성은 다음 프레임 경계로 미룬다.
			m_SwapChainDirty = true;
		}
		else if (present != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("vkQueuePresentKHR 실패!");
		}

		// 3. 다음 슬롯으로 회전
		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	VulkanContext::~VulkanContext()
	{
		if (m_LogicalDevice == VK_NULL_HANDLE) return;

		// 상위가 이미 정리됐다는 전제지만, 파괴 직전 한 번 더 막는다 (비용은 종료 시 1회)
		vkDeviceWaitIdle(m_LogicalDevice);

		// 1. 동기화 객체
		for (VkSemaphore sem : m_ImageAvailableSemaphores) {
			if (sem != VK_NULL_HANDLE) vkDestroySemaphore(m_LogicalDevice, sem, nullptr);
		}
		for (VkSemaphore sem : m_RenderFinishedSemaphores) {
			if (sem != VK_NULL_HANDLE) vkDestroySemaphore(m_LogicalDevice, sem, nullptr);
		}
		for (VkFence fence : m_InFlightFences) {
			if (fence != VK_NULL_HANDLE) vkDestroyFence(m_LogicalDevice, fence, nullptr);
		}
		m_ImageAvailableSemaphores.clear();
		m_RenderFinishedSemaphores.clear();
		m_InFlightFences.clear();
		m_ImagesInFlight.clear();   // 프레임 슬롯 펜스를 빌려 쓰던 배열 — 개별 파괴 대상 아님

		// 2. 커맨드풀 (커맨드버퍼는 풀과 함께 해제되므로 개별 free 불필요)
		if (m_GraphicsCommandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_LogicalDevice, m_GraphicsCommandPool, nullptr);
		if (m_TransferCommandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_LogicalDevice, m_TransferCommandPool, nullptr);
		m_CommandBuffers.clear();

		// 3. 스왑체인 (뷰 + 핸들)
		VulkanSwapChain::DestroySwapChain(m_LogicalDevice, m_SwapChainBundle);

		// 4. VMA — 디바이스보다 먼저. 남은 할당이 있으면 여기서 경고가 뜬다
		if (m_Allocator != VK_NULL_HANDLE) vmaDestroyAllocator(m_Allocator);

		// 5. 디바이스 → 서피스 → 인스턴스 (생성의 역순)
		vkDestroyDevice(m_LogicalDevice, nullptr);
		m_LogicalDevice = VK_NULL_HANDLE;

		if (m_Surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		if (m_Instance != VK_NULL_HANDLE) vkDestroyInstance(m_Instance, nullptr);

		s_VulkanContext = nullptr;
		APP_WMS_CORE_INFO("VulkanContext destroyed");
	}

	VulkanContext* VulkanContext::Get()
	{
		return s_VulkanContext;
	}

}
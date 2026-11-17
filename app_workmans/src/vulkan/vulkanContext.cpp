#include "pch.h"
#include "vulkanContext.h"
#include <GLFW/glfw3.h>   // framebuffer size query, used to detect minimization

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

		//  Create the Vulkan instance handle
		m_Instance = VulkanInit::CreateInstance();

		//  Create the surface that connects the OS window to Vulkan
		m_Surface = VulkanInit::CreateSurface(m_Instance, m_WindowHandle);

		//  Pick the actual GPU installed in the machine (PhysicalDevice)
		m_PhysicalDevice = VulkanInit::PickPhysicalDevice(m_Instance);

		// vk_init
		m_QueueIndices = VulkanInit::FindQueueFamilies(m_PhysicalDevice, m_Surface);

		//  Inject the tunable graphics settings and create the logical device
		VulkanInit::EngineGraphicsSettings settings;
		m_LogicalDevice = VulkanInit::CreateLogicalDevice(m_PhysicalDevice, m_QueueIndices, settings);
		//  Pull the three real queue handles out of the logical device
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.presentFamily.value(), 0, &m_PresentQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueIndices.transferFamily.value(), 0, &m_TransferQueue);

		APP_WMS_CORE_INFO("Queue family - graphics:{0}, present:{1}, transfer:{2} (dedicated transfer: {3})",
			m_QueueIndices.graphicsFamily.value(),
			m_QueueIndices.presentFamily.value(),
			m_QueueIndices.transferFamily.value(),
			m_QueueIndices.hasDedicatedTransfer());

		//  Create the VMA allocator (right after instance/physical/logical device are all in hand)
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		allocatorInfo.instance = m_Instance;
		allocatorInfo.physicalDevice = m_PhysicalDevice;
		allocatorInfo.device = m_LogicalDevice;
		if (vmaCreateAllocator(&allocatorInfo, &m_Allocator) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("Failed VMA Allocator !");
		}
		//  Create the two command pools
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
		// 1. Graphics command buffers (one per frame in flight)
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_GraphicsCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		if (vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("commandBuffer assigning failed!");
		}

		// 2. Sync objects tied to the frame slots (fixed count -> never recreated)
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semInfo{};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // so the first frame's waitFence passes immediately

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkCreateSemaphore(m_LogicalDevice, &semInfo, nullptr, &m_ImageAvailableSemaphores[i]);
			vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_InFlightFences[i]);
		}

		// 3. The ones tied to the image count -- split out because that count can change on swapchain recreate
		recreateImageSyncObjects();

		APP_WMS_CORE_INFO("Frame resources: cmdBuf {0}, imageAvailable/fence {0}, renderFinished {1}",
			MAX_FRAMES_IN_FLIGHT, m_SwapChainBundle.images.size());
	}

	void VulkanContext::recreateImageSyncObjects()
	{
		// renderFinished is the signal present waits on, so it has to be per-image.
		// Per-frame-slot would not line up with the image count, letting two presents share one semaphore.
		for (VkSemaphore sem : m_RenderFinishedSemaphores) {
			if (sem != VK_NULL_HANDLE) vkDestroySemaphore(m_LogicalDevice, sem, nullptr);
		}

		const size_t imageCount = m_SwapChainBundle.images.size();
		m_RenderFinishedSemaphores.assign(imageCount, VK_NULL_HANDLE);
		// No fences are created here -- this array only borrows the frame-slot fences to track them
		m_ImagesInFlight.assign(imageCount, VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semInfo{};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (size_t i = 0; i < imageCount; i++) {
			vkCreateSemaphore(m_LogicalDevice, &semInfo, nullptr, &m_RenderFinishedSemaphores[i]);
		}
	}

	bool VulkanContext::recreateSwapChain()
	{
		// When minimized the framebuffer size becomes 0. A swapchain cannot be built with extent 0,
		// so we back off and keep the stale mark until it is restored (App::run skips the frame).
		int w = 0, h = 0;
		glfwGetFramebufferSize(m_WindowHandle, &w, &h);
		if (w == 0 || h == 0) {
			return false;
		}

		// The GPU may still be using the old swapchain and views. Guarded the same way as decision #3.
		vkDeviceWaitIdle(m_LogicalDevice);

		VulkanSwapChain::DestroySwapChain(m_LogicalDevice, m_SwapChainBundle);

		m_SwapChainBundle = VulkanSwapChain::CreateSwapChain(m_PhysicalDevice, m_LogicalDevice,
			m_Surface, m_WindowHandle, m_QueueIndices);

		if (m_SwapChainBundle.handle == VK_NULL_HANDLE) {
			APP_WMS_CORE_ERROR("SwapChain recreation failed!");
			return false;
		}

		recreateImageSyncObjects();

		// The old image index / slot no longer mean anything
		m_CurrentFrame = 0;
		m_CurrentImageIndex = 0;
		m_SwapChainDirty = false;
		return true;
	}

	bool VulkanContext::beginFrame()
	{
		// 1. CPU waits until this slot's previous GPU work is done (makes command buffer reuse safe)
		vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		// 2. Acquire the next swapchain image -- the imageAvailable semaphore signals completion
		VkResult acquire = vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChainBundle.handle,
			UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImageIndex);

		if (acquire == VK_ERROR_OUT_OF_DATE_KHR) {
			// Do not recreate right here -- the layer above handles it at the next frame boundary.
			m_SwapChainDirty = true;
			return false;
		}
		if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) {
			APP_WMS_CORE_ERROR("vkAcquireNextImageKHR failed!");
			return false;
		}

		// 3. If another frame is still using the acquired image on the GPU, wait on that fence too
		if (m_ImagesInFlight[m_CurrentImageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_LogicalDevice, 1, &m_ImagesInFlight[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);
		}
		m_ImagesInFlight[m_CurrentImageIndex] = m_InFlightFences[m_CurrentFrame];

		// 4. Submission is settled -> reset the fence, then begin the command buffer
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

		// 1. Submit: wait on imageAvailable (color output stage) -> when drawing is done, signal renderFinished + the fence
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
			APP_WMS_CORE_ERROR("vkQueueSubmit failed!");
			return;
		}

		// 2. Put it on screen: present after waiting on renderFinished
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
			// This frame has already been submitted. Defer recreation to the next frame boundary.
			m_SwapChainDirty = true;
		}
		else if (present != VK_SUCCESS) {
			APP_WMS_CORE_ERROR("vkQueuePresentKHR failed!");
		}

		// 3. Rotate to the next slot
		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	VulkanContext::~VulkanContext()
	{
		if (m_LogicalDevice == VK_NULL_HANDLE) return;

		// The layers above are assumed cleaned up already, but guard once more right before destruction (cost: once at shutdown)
		vkDeviceWaitIdle(m_LogicalDevice);

		// 1. Sync objects
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
		m_ImagesInFlight.clear();   // this array only borrowed the frame-slot fences -- nothing to destroy individually

		// 2. Command pools (command buffers are freed with the pool, so no individual free)
		if (m_GraphicsCommandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_LogicalDevice, m_GraphicsCommandPool, nullptr);
		if (m_TransferCommandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_LogicalDevice, m_TransferCommandPool, nullptr);
		m_CommandBuffers.clear();

		// 3. Swapchain (views + handle)
		VulkanSwapChain::DestroySwapChain(m_LogicalDevice, m_SwapChainBundle);

		// 4. VMA -- before the device. Any leftover allocation warns right here
		if (m_Allocator != VK_NULL_HANDLE) vmaDestroyAllocator(m_Allocator);

		// 5. Device -> surface -> instance (reverse of creation)
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
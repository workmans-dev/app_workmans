#pragma once
#include "renderer/graphicContext.h"
#include <vulkan/vulkan.h> 
#include "vk_mem_alloc.h"

#include "vulkan/vk_swapChain.h"
#include "vulkan/vk_init.h"


struct GLFWwindow;

namespace AppWMs {

	class VulkanContext : public GraphicContext {
	public:
		VulkanContext(GLFWwindow* windowHandle);
		// WARNING: destroys in the exact reverse order of creation. It must be called only after
		//    the higher-level resources (RenderTarget/pipeline/ImGui backend/RendererAPI) are all released -- see App::~App.
		virtual ~VulkanContext();

		virtual void init() override;
		// begin
		virtual bool beginFrame() override;
		// end
		virtual void swapBuffers() override;

		// The resize path.
		//   A window resize, or OUT_OF_DATE from acquire/present, only leaves a "stale" mark;
		//   the actual recreation is driven by VulkanRendererAPI::beginFrame at the frame boundary,
		//   because swapping it mid-frame corrupts the command buffer being recorded.
		void requestSwapChainRecreate() { m_SwapChainDirty = true; }
		bool isSwapChainDirty() const { return m_SwapChainDirty; }
		// When minimized (extent 0) it does nothing and returns false, deferring until restored.
		bool recreateSwapChain();

		static VulkanContext* Get();
		// Number of frame-in-flight slots (the GPU works on the current frame while the CPU prepares the next)
		static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;


		// Getters so the backend renderer (VulkanRendererAPI) can bind hardware info cleanly
		// from this context without back-referencing the layers above it
		VkInstance getInstance() const { return m_Instance; }
		VkSurfaceKHR getSurface() const { return m_Surface; }
		VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice getDevice() const { return m_LogicalDevice; }
		// momory allocater
		VmaAllocator getAllocator() const { return m_Allocator; }
		// coming from vk_init, QueueFamilyIndices
		VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue getPresentQueue() const { return m_PresentQueue; }
		VkQueue getTransferQueue() const { return m_TransferQueue; }

		VkCommandPool getGraphicsCommandPool() const { return m_GraphicsCommandPool; }
		VkCommandPool getTransferCommandPool() const { return m_TransferCommandPool; }

		const VulkanInit::QueueFamilyIndices& getQueueIndices() const { return m_QueueIndices; }

		// WARNING: the bundle is the single source of truth. This used to return a separate
		//    m_SwapChain member that was never assigned, so it was always VK_NULL_HANDLE (unnoticed because unused).
		VkSwapchainKHR getSwapChain() const { return m_SwapChainBundle.handle; }

		// Individual vk_swapChain getters
		VkFormat getSwapChainImageFormat() const { return m_SwapChainBundle.format; }
		VkExtent2D getSwapChainExtent()      const { return m_SwapChainBundle.extent; }
		const std::vector<VkImage>& getSwapChainImages() const { return m_SwapChainBundle.images; }
		const std::vector<VkImageView>& getSwapChainImageViews() const { return m_SwapChainBundle.imageViews; }

		// Where layers borrow this frame's command buffer (keeps Vulkan from leaking into the layers)
		VkCommandBuffer getCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }
		uint32_t        getCurrentImageIndex()   const { return m_CurrentImageIndex; }
		uint32_t        getCurrentFrameIndex()   const { return m_CurrentFrame; }

	private:
		GLFWwindow* m_WindowHandle;

		// context pointer 
		static VulkanContext* s_VulkanContext;
		// vulkan core objects
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		// memory
		VmaAllocator m_Allocator = VK_NULL_HANDLE;

		// from vk_init
		VulkanInit::QueueFamilyIndices m_QueueIndices;
		// the three queue handles
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		VkQueue m_TransferQueue = VK_NULL_HANDLE;
		// the two command pools
		VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_TransferCommandPool = VK_NULL_HANDLE;

		// frame resources
		std::vector<VkCommandBuffer> m_CommandBuffers;              // [frame] for recording graphics
		std::vector<VkSemaphore>     m_ImageAvailableSemaphores;    // [frame] acquire-complete signal
		std::vector<VkSemaphore>     m_RenderFinishedSemaphores;    // [image] render-complete signal (present waits on it)
		std::vector<VkFence>         m_InFlightFences;              // [frame] CPU waits here before reusing the slot
		std::vector<VkFence>         m_ImagesInFlight;              // [image] tracks the in-use fence per image
		uint32_t m_CurrentFrame = 0;
		uint32_t m_CurrentImageIndex = 0;

		// from vk_swapChain
		VulkanSwapChain::SwapChainBundle m_SwapChainBundle;
		bool m_SwapChainDirty = false;   // marks a window resize / OUT_OF_DATE detection

		// Creates the command buffers and sync objects in one go (last step of init)
		void createFrameResources();
		// Recreates only the sync objects tied to the image count (which can change on recreate)
		void recreateImageSyncObjects();

	};

}
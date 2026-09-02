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
		// ⚠️ 생성의 정확한 역순으로 파괴한다. 상위 리소스(FrameBuffer/파이프라인/
		//    ImGui 백엔드/RendererAPI)가 모두 반납된 뒤에 불려야 한다 — App::~App 참고.
		virtual ~VulkanContext();

		virtual void init() override;
		// begin
		virtual bool beginFrame() override;
		// end
		virtual void swapBuffers() override;

		// 🟢 리사이즈 경로.
		//   창 크기 변경이나 acquire/present의 OUT_OF_DATE는 "낡음" 표시만 남기고,
		//   실제 재생성은 상위(VulkanRendererAPI::beginFrame)가 프레임 경계에서 구동한다.
		//   프레임 한복판에서 갈아끼우면 기록 중인 커맨드버퍼가 깨지기 때문.
		void requestSwapChainRecreate() { m_SwapChainDirty = true; }
		bool isSwapChainDirty() const { return m_SwapChainDirty; }
		// 최소화(extent 0)면 아무것도 안 하고 false를 반환한다. 복원될 때까지 미룬다.
		bool recreateSwapChain();

		static VulkanContext* Get();
		// 🟢 frame-in-flight 슬롯 수 (CPU가 다음 프레임 준비하는 동안 GPU가 현재 프레임 처리)
		static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;


		// 🟢 하위 렌더러 구현체(VulkanRendererAPI)가 상위 레이어를 역참조하지 않고
		// 이 컨텍스트에서 하드웨어 정보를 깨끗하게 바인딩해 갈 수 있도록 게터 제공
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

		// ⚠️ 번들이 유일한 원본이다. 예전엔 별도 m_SwapChain 멤버를 반환했는데
		//    한 번도 대입되지 않아 항상 VK_NULL_HANDLE이었다 (미사용이라 안 드러났음).
		VkSwapchainKHR getSwapChain() const { return m_SwapChainBundle.handle; }

		// 🟢 vk_swapChain 개별 게터 추가
		VkFormat getSwapChainImageFormat() const { return m_SwapChainBundle.format; }
		VkExtent2D getSwapChainExtent()      const { return m_SwapChainBundle.extent; }
		const std::vector<VkImage>& getSwapChainImages() const { return m_SwapChainBundle.images; }
		const std::vector<VkImageView>& getSwapChainImageViews() const { return m_SwapChainBundle.imageViews; }

		// 🟢 레이어들이 이번 프레임 커맨드버퍼를 빌려가는 창구 (벌칸이 레이어로 안 샘)
		VkCommandBuffer getCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }
		uint32_t        getCurrentImageIndex()   const { return m_CurrentImageIndex; }
		uint32_t        getCurrentFrameIndex()   const { return m_CurrentFrame; }

	private:
		GLFWwindow* m_WindowHandle;

		// context pointer 
		static VulkanContext* s_VulkanContext;
		// vulkan 코어 객체들
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		// memory
		VmaAllocator m_Allocator = VK_NULL_HANDLE;

		// from vk_init
		VulkanInit::QueueFamilyIndices m_QueueIndices;
		// 🟢 큐 핸들 3종
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		VkQueue m_TransferQueue = VK_NULL_HANDLE;
		// 🟢 커맨드풀 2종
		VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_TransferCommandPool = VK_NULL_HANDLE;

		// 🟢 프레임 리소스
		std::vector<VkCommandBuffer> m_CommandBuffers;              // [frame] 그래픽스 기록용
		std::vector<VkSemaphore>     m_ImageAvailableSemaphores;    // [frame] acquire 완료 신호
		std::vector<VkSemaphore>     m_RenderFinishedSemaphores;    // [image] 렌더 완료 신호 (present 대기)
		std::vector<VkFence>         m_InFlightFences;              // [frame] CPU가 슬롯 재사용 전 대기
		std::vector<VkFence>         m_ImagesInFlight;              // [image] 이미지별 사용중 펜스 추적
		uint32_t m_CurrentFrame = 0;
		uint32_t m_CurrentImageIndex = 0;

		// from vk_swapChain
		VulkanSwapChain::SwapChainBundle m_SwapChainBundle;
		bool m_SwapChainDirty = false;   // 창 리사이즈 / OUT_OF_DATE 감지 표시

		// 🟢 커맨드버퍼 + 동기화 객체 일괄 생성 (init 마지막 단계)
		void createFrameResources();
		// 🟢 이미지 개수에 묶인 동기화 객체만 다시 만든다 (재생성 시 개수가 바뀔 수 있음)
		void recreateImageSyncObjects();

	};

}
#include "pch.h"
#include "imGui/imGuiLayer.h"
#include "imgui.h"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "core/application.h"

#include "vulkan/vulkanContext.h"
#include "vulkan/vulkanRenderer.h"
#include "renderer/rendererCommand.h"
#define GLM_ENABLE_EXPERMENTAL

#include "ImGuizmo.h"


namespace AppWMs {

	static void setDark() {
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Backgrounds
		colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f); // #262626 (Main window background)
		colors[ImGuiCol_ChildBg] = ImVec4 (0.12f, 0.12f, 0.12f, 1.0f); // Child window background
		colors[ImGuiCol_PopupBg] = ImVec4 (0.12f, 0.12f, 0.12f, 1.0f); // Popup background
		colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // Border color

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.933f, 0.933f, 0.933f, 1.0f); // #E0E0E0 (Main text color)
		colors[ImGuiCol_TextDisabled] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // #7A7A7A (Disabled text)
		colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Separator color

		// Buttons
		colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f); // #3390D2 (Button background)
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f); // #43B5B1 (Hovered button)
		colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f); // #3390D2 (Active button)

		// Frame
		colors[ImGuiCol_FrameBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // #393939 (Frame background)
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.318f, 0.459f, 0.757f, 1.0f); // #51A5C1 (Hovered frame background)
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.204f, 0.357f, 0.525f, 1.0f); // #3390D2 (Active frame background)


		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.168f, 0.168f, 0.168f, 1.0f); // Inactive tab background (darker gray)
		colors[ImGuiCol_TabHovered] = ImVec4(0.318f, 0.459f, 0.757f, 1.0f); // #40A9BB (Hovered tab, light blue)
		colors[ImGuiCol_TabActive] = ImVec4(0.118f, 0.118f, 0.118f, 1.0f); // Active tab background (darker)
		colors[ImGuiCol_TabSelected] = ImVec4(0.204f, 0.357f, 0.525f, 1.0f); 
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f); // Active tab background (darker)
		//colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.152f, 0.152f, 0.152f, 1.0f); // Active tab background (darker)

		// Menu
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f); // #262626 (Menu bar background)
		colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f); // #393939 (Menu header background)
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.318f, 0.459f, 0.757f, 1.0f); // #40A9BB (Hovered header)
		colors[ImGuiCol_HeaderActive] = ImVec4(0.204f, 0.357f, 0.525f, 1.0f); // #3390D2 (Active header)

		// Misc
		colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // Transparent shadow

		// Adjust some style variables
		style.WindowRounding = 5.0f;
		style.FrameRounding = 2.0f;
		style.ScrollbarRounding = 5.0f;
		style.GrabRounding = 4.0f;
		style.PopupRounding = 4.0f;

	}

	
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}
	ImGuiLayer::~ImGuiLayer() {}

	void ImGuiLayer::onAttach()
	{
		// 1. Only creates a pure ImGui context and does the basic setup (keeps it independent)
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/static/Rubik-Regular.ttf", 14.6f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/static/Rubik-Bold.ttf", 14.6f);

		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		setDark();

		// The old messy App casting and forced window scan (App::getApp()...) was removed
		// entirely, in line with the design principle.
	}

	void ImGuiLayer::InitVulkanBackend(GLFWwindow* window)
	{
		// 1. Window hookup
		ImGui_ImplGlfw_InitForVulkan(window, true);

		// 2. Pull out the hardware infrastructure (Context)
		auto context = VulkanContext::Get();

		// 3. Clean downcast hookup through pm_rendererAPI_ptr
		auto renderer = static_cast<VulkanRendererAPI*>(&RendererCommand::getAPI());

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.ApiVersion = VK_API_VERSION_1_3;  // required field in the new version
		init_info.Instance = context->getInstance();
		init_info.PhysicalDevice = context->getPhysicalDevice();
		init_info.Device = context->getDevice();

		// Reuses the index Context found once during init (removes the hardcoded 0)
		uint32_t graphicsFamily = context->getQueueIndices().graphicsFamily.value();
		init_info.QueueFamily = graphicsFamily;
		vkGetDeviceQueue(context->getDevice(), graphicsFamily, 0, &init_info.Queue);
		// struct ImGui_ImplVulkan_InitInfo set
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = renderer->getDescriptorPool();

		init_info.MinImageCount = 2;
		init_info.ImageCount = (uint32_t)context->getSwapChainImages().size();

		init_info.Allocator = nullptr;
		// struct ImGui_ImplVulkan_PipelineInfo set
		
		init_info.PipelineInfoMain.RenderPass = renderer->getRenderPass();
		init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		
		ImGui_ImplVulkan_Init(&init_info);
	}

	void ImGuiLayer::onEvent(Event& event)
	{
		if (!pm_eventBlocking) {
			ImGuiIO& io = ImGui::GetIO();
			event.p_handled |= event.isInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
			event.p_handled |= event.isInCategory(EventCategoryMouse) & io.WantCaptureMouse;
		}
	}

	void ImGuiLayer::onDetach()
	{
		// Shutdown handling, cleaned up to be Vulkan-backend only
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::begin() {
		// Declares the Vulkan backend frame at the start of every frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::end() {
		ImGuiIO& io = ImGui::GetIO();
		App& app = App::getApp();
		io.DisplaySize = ImVec2((float)app.getWindow().getWidth(), (float)app.getWindow().getHeight());

		ImGui::Render();

		// Records the ImGui draws into the main window swapchain render pass
		//    The command buffer is borrowed from the one Context already opened in beginFrame
		auto ctx = VulkanContext::Get();
		auto renderer = static_cast<VulkanRendererAPI*>(&RendererCommand::getAPI());
		VkCommandBuffer cmd = ctx->getCurrentCommandBuffer();

		const glm::vec4& cc = renderer->getClearColor();
		VkClearValue clearValue{};
		clearValue.color = { { cc.r, cc.g, cc.b, cc.a } };  // attachment 0 = color (no depth)

		VkRenderPassBeginInfo rpBegin{};
		rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBegin.renderPass = renderer->getRenderPass();
		rpBegin.framebuffer = renderer->getFramebuffer(ctx->getCurrentImageIndex());
		rpBegin.renderArea.offset = { 0, 0 };
		rpBegin.renderArea.extent = ctx->getSwapChainExtent();
		rpBegin.clearValueCount = 1;
		rpBegin.pClearValues = &clearValue;

		vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
		vkCmdEndRenderPass(cmd);

		// Multi-viewport window handling
		// WARNING: for secondary viewports ImGui handles its own swapchain/command buffer/submit/present.
		//    The main command buffer is still being recorded (unsubmitted), so there is no queue conflict.
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}
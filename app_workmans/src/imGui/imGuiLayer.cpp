#include "pch.h"
#include "imGui/imGuiLayer.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include "core/application.h"

#include "GLFW/glfw3.h"
#include "glad/glad.h"
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

	
	ImGuiLayer::ImGuiLayer():Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::onAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/static/Rubik-Regular.ttf", 14.6f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/static/Rubik-Bold.ttf", 14.6f);

		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 	
		// dark theme
		setDark();
	
		App& app = App::getApp();
		// 형님의 설계 원칙에 따라 완전히 삭제되어 쓰레기통으로 들어갔습니다.

		ImGui_ImplGlfw_InitForOpenGL(glfw_window_ptr, true);
		ImGui_ImplOpenGL3_Init("#version 410");
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
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}




	void ImGuiLayer::begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}


	void ImGuiLayer::end() {
		ImGuiIO& io = ImGui::GetIO();
		App& app = App::getApp();
		io.DisplaySize = ImVec2((float)app.getWindow().getWidth(), (float)app.getWindow().getHeight());
		ImGui::Render();



		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);

		
	}

}
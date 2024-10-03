#include "pch.h"
#include "imgui.h"
#include "imGui/imGuiLayer.h"
#include "debugs/ImGuiRenderer.h"

#include "GLFW/glfw3.h"
#include "application.h"
#include "glad/glad.h"


namespace AppWMs {
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
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;           // Tab key
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;   // Left arrow key
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT; // Right arrow key
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;       // Up arrow key
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;   // Down arrow key
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;   // Page Up key
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN; // Page Down key
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;        // Home key
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;          // End key
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;    // Insert key
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;    // Delete key
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE; // Backspace key
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;      // Space key
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;      // Enter key
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;    // Escape key

		ImGui_ImplOpenGL3_Init("#version 410");


	}
	void ImGuiLayer::onDetach()
	{
	}
	void ImGuiLayer::onUpdate()
	{
		App& app = App::getApp();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(app.getWindow().getWidth(), app.getWindow().getHeight());

		float time = (float)glfwGetTime();
		io.DeltaTime = m_time > 0.0f ? (time - m_time) : (1.0f / 60.0f);
		m_time = time;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
	}

	void ImGuiLayer::onEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MousePressedEvent>(BIND_EVENT_FN(ImGuiLayer::onMousePressedEvent));
		dispatcher.Dispatch<MouseReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseMovedEvent));
		dispatcher.Dispatch<MouseScrollEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseScrollEvent));

		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyReleasedEvent));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyPressedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyTypedEvent));

		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(ImGuiLayer::onWindowResizeEvent));

	}

	bool ImGuiLayer::onMousePressedEvent(MousePressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();

		io.MouseDown[e.get_mouseButton()] = true;
		return false;
	}

	bool ImGuiLayer::onMouseReleasedEvent(MouseReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();

		io.MouseDown[e.get_mouseButton()] = false;
		return false;
	}

	bool ImGuiLayer::onMouseMovedEvent(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();

		io.MousePos = ImVec2(e.get_xPnt(), e.get_yPnt());
		return false;
	}

	bool ImGuiLayer::onMouseScrollEvent(MouseScrollEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();

		io.MouseWheelH += e.get_xScroll();
		io.MouseWheel += e.get_yScroll();
		return false;
	}

	bool ImGuiLayer::onKeyReleasedEvent(KeyReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.getKeyCode()] = false;
		return false;
	}

	bool ImGuiLayer::onKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[e.getKeyCode()] = true;
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
		return false;
	}

	bool ImGuiLayer::onKeyTypedEvent(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int keycode = e.getKeyCode();
		if (keycode >0 && keycode < 0x10000)
			io.AddInputCharacter((unsigned short)keycode);
		return false;
	}

	bool ImGuiLayer::onWindowResizeEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(e.getWidth(), e.getHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		glViewport(0, 0, e.getWidth(), e.getHeight());

		return false;
	}

}
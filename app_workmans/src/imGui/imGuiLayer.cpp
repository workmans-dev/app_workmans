#include "pch.h"
#include "imGui/imGuiLayer.h"
#include "imgui.h"
#include "debugs/ImGuiRenderer.h"
#include "GLFW/glfw3.h"
#include "application.h"

namespace AppWMs {
	ImGuiLayer::ImGuiLayer():Layer("ImGuiLayer")
	{
	}
	ImGuiLayer::~ImGuiLayer()
	{
	}
	void ImGuiLayer::onAttach()
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		ImGui_ImplOpenGL3_Init("#version 410");


	}
	void ImGuiLayer::onDetach()
	{
	}
	void ImGuiLayer::onUpdate()
	{

		ImGuiIO& io = ImGui::GetIO();
		App& app = App::getApp();
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
	void ImGuiLayer::onEvent(Event& e)
	{
	}
}
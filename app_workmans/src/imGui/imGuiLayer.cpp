#include "imGui/imGuiLayer.h"
#include "debugs/imGuiRenderer.h"
#include "GLFW/glfw3.h"

namespace AppWMs {
	ImGuiLayer::ImGuiLayer():Layer("ImGuiLayer")
	{
	}
	ImGuiLayer::~ImGuiLayer()
	{
	}
	void ImGuiLayer::onAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		ImGui_ImplOpenGL3_Init("#version 416");


	}
	void ImGuiLayer::onDetach()
	{
	}
	void ImGuiLayer::onUpdate()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	void ImGuiLayer::onEvent(Event& e)
	{
	}
}
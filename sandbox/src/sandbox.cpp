#include "app_workmans.h"
#include "imgui/imgui.h"
#include <iostream>

class ExampleLayer:public AppWMs::Layer
{
public:
	ExampleLayer() :Layer("Example") {
	}
	void onUpdate() override{
		//APP_WMS_CORE_INFO("ExampleLayer : updated");
	}
	void onEvent(AppWMs::Event& event) override{
		if (event.getEventType() == AppWMs::EventType::KeyPressed) {
			AppWMs::KeyPressedEvent& e = (AppWMs::KeyPressedEvent&) event;
			APP_WMS_CLI_TRACE("{0}", (char)e.getKeyCode());
		}
	}
	virtual void onImGuiRender() override {
		ImGui::Begin("test");
		ImGui::Text("APPPPPPPP");
		ImGui::End();
	}
	//void onAttach() override {
	//	std::cout << "test" << std::endl;
	//}
};

class Sandbox : public AppWMs::App
{
public:
	Sandbox() {
		std::cout << "created SandBox---- parent class ----adding Layers" << std::endl;
		pushLayer(new ExampleLayer());
	}
	~Sandbox() {

	}

};

AppWMs::App* AppWMs::createApplication(){
	return new Sandbox();
}
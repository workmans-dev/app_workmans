#include "app_workmans.h"
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
		APP_WMS_CORE_TRACE("{0}", event);
	}
};

class Sandbox : public AppWMs::App
{
public:
	Sandbox() {
		std::cout << "created SandBox" << std::endl;
		pushLayer(new ExampleLayer());
		pushOverlay(new AppWMs::ImGuiLayer());
	}
	~Sandbox() {

	}


};

AppWMs::App* AppWMs::createApplication(){
	return new Sandbox();
}
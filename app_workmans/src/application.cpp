#include "pch.h"
#include "core.h"
#include "application.h"

#include "events/applicationEvent.h"


namespace AppWMs {
#define BIND_EVENT_FN(x) std::bind(&App::x, this, std::placeholders::_1)
	App::App() {
		pm_GLFWwindow_ptr = std::unique_ptr<Window>(Window::create());
		// set event call back
		pm_GLFWwindow_ptr->setEventCallBack(BIND_EVENT_FN(onEvent));
	}

	App::~App() {
	}

	void App::onEvent(Event& e) {
		APP_WMS_CORE_INFO("{0}", e)
	}

	void App::run() {
		while (pm_running) {
			pm_GLFWwindow_ptr->onUpdate();
		}
	}


}
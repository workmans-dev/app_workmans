#include "pch.h"
#include "core.h"
#include "application.h"
#include <glad/glad.h>


namespace AppWMs {
#define BIND_EVENT_FN(x) std::bind(&App::x, this, std::placeholders::_1)
	App::App() {
		pm_GLFWwindow_ptr = std::unique_ptr<Window>(Window::create());
		// set event call back
		pm_GLFWwindow_ptr->setEventCallBack(BIND_EVENT_FN(onEvent));

	}

	App::~App() {
	}

	void App::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));
		for (auto it = m_layerStack.end(); it != m_layerStack.begin();)
		{
			(*--it)->onEvent(event);
			if (event.p_handled)
				break;
		}
	}

	void App::run() {
		while (pm_running) {
			for (Layer* layer : m_layerStack) {
				layer->onUpdate();
			}
			pm_GLFWwindow_ptr->onUpdate();
		}
	}

	bool App::onWindowClose(WindowCloseEvent& e) {
		pm_running = false;
		return true;
	}

	void App::pushLayer(Layer* layer) {
		m_layerStack.pushLayer(layer);
	}

	void App::pushOverlay(Layer* overlay) {
		m_layerStack.pushOverlay(overlay);
	}

}
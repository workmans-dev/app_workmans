#include "pch.h"
#include "core.h"
#include "application.h"
#include <glad/glad.h>
#include "input.h"

namespace AppWMs {
	App* App::m_Instance = nullptr;

	App::App() {
		APP_WMS_CORE_ASSERTS(!m_Instance, "Exits!!!")
		m_Instance = this;
		std::cout << "created the App - Base class" << std::endl;
		// the Type is Window BUT it will return as the Win_win which is a parent Class of Window.
		// don't be confused here, so important!!
		pm_window_ptr = std::unique_ptr<Window>(Window::create());
		// set event call back
		pm_window_ptr->setData_EventCallBack(BIND_EVENT_FN(App::onEvent));

	}

	App::~App() {
	}

	void App::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(App::onWindowClose));
		for (auto it = m_layerStack.end(); it != m_layerStack.begin();)
		{
			(*--it)->onEvent(event);
			if (event.p_handled)
				break;
		}
	}

	void App::run() {
		while (pm_running) 
		{
			for (Layer* layer : m_layerStack) 
			{
				//glClearColor(1, 0, 1, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				layer->onUpdate();
			}
			//auto [x, y] = Input::getMousePos();
			//APP_WMS_CORE_TRACE("{0}, {1}", x, y);
			pm_window_ptr->onUpdate();
		}
	}

	bool App::onWindowClose(WindowCloseEvent& e) {
		pm_running = false;
		return true;
	}

	void App::pushLayer(Layer* layer) {
		m_layerStack.pushLayer(layer);
		layer->onAttach();
	}

	void App::pushOverlay(Layer* overlay) {
		m_layerStack.pushOverlay(overlay);
		overlay->onAttach();
	}

}
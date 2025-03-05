#include "pch.h"
#include "application.h"
#include "renderer/renderer.h"
#include "core/input.h"
#include "core/timer.h"
#include <glfw/glfw3.h>

namespace AppWMs {
	App* App::pm_Instance = nullptr;

	App::App() {
		APP_WMS_PROFILE_FUNC();
		APP_WMS_CORE_ASSERTS(!pm_Instance, "Exits!!!")
		pm_Instance = this;
		// the Type is Window BUT it will return as the Win_win which is a parent Class of Window.
		// don't be confused here, so important!!
		pm_window_ptr = std::unique_ptr<Window>(Window::create());
		// set event call back
		pm_window_ptr->setData_EventCallBack(BIND_EVENT_FN(App::onEvent));

		// renderer init
		Renderer::init();

		// GUI
		pm_imGuiLayer_ptr = new ImGuiLayer();
		pushOverlay(pm_imGuiLayer_ptr);

	}

	App::~App() {

	}

	// send all events of layer events to Win_window : pm_window_ptr
	void App::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(App::onWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(App::onWindowResize));
		for (auto it = pm_layerStack.begin(); it != pm_layerStack.end();++it)
		{
			(*it)->onEvent(event);
			if (event.p_handled)
				break;
		}
	}

	void App::run() {
		while (pm_running) 
		{
			float time = (float)glfwGetTime();
			Timer timer = time - pm_dt;
			pm_dt = timer;

			if (!pm_minimized) {
				// layer base class onUpdate
				for (Layer* layer : pm_layerStack) 
					layer->onUpdate(timer);
			}

			// gui layer
			pm_imGuiLayer_ptr->begin();
			for (Layer* layer : pm_layerStack)
			{
				layer->onImGuiRender();
			}
			pm_imGuiLayer_ptr->end();

			// final update for application window
			pm_window_ptr->onUpdate();


		}
	}

	bool App::onWindowClose(WindowCloseEvent& e) {
		pm_running = false;
		return true;
	}

	bool App::onWindowResize(WindowResizeEvent& e)
	{
		uint32_t w = e.getWidth();
		uint32_t h = e.getHeight();
		if (w==0 || h==0) {
			pm_minimized = true;
		}
		pm_minimized = false;
		Renderer::onWindowResize(w, h);
		return false;
	}

	bool App::onWindowCameraAim(MousePressedEvent& mouse_event, KeyPressedEvent& key_event)
	{
		if (mouse_event.get_mouseButton() == GLFW_MOUSE_BUTTON_LEFT &&
			key_event.getKeyCode()==GLFW_KEY_LEFT_ALT) {
			pm_start_measure = true;
			
		}
		pm_start_measure = false;
		return false;
	}

	void App::pushLayer(Layer* layer) {
		pm_layerStack.pushLayer(layer);
		layer->onAttach();
	}

	void App::pushOverlay(Layer* overlay) {
		pm_layerStack.pushOverlay(overlay);
		overlay->onAttach();
	}
	void App::close() {
		pm_running = false;
	}
}
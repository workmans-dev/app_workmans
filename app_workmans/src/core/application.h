#pragma once

#include "core/core.h"
#include "window.h"
#include "layerStack.h"
#include "events/event.h"
#include "events/applicationEvent.h"
#include "imGui/imGuiLayer.h"

namespace AppWMs {

	class App
	{
	public:
		App();
		virtual ~App();
		void onEvent(Event& e);
		void run();

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);
		void close();
		inline Window& getWindow() { return *pm_window_ptr; }
		inline static App& getApp() { return *pm_Instance; }
		ImGuiLayer* get_ImGuiLayer() { return pm_imGuiLayer_ptr; }

	private:
		bool onWindowClose(WindowCloseEvent& e);
		bool onWindowResize(WindowResizeEvent& e);
		bool onWindowCameraAim(MousePressedEvent& mouse_event, KeyPressedEvent& key_event);
	private:
		Scope<Window> pm_window_ptr;
		ImGuiLayer* pm_imGuiLayer_ptr;
		LayerStack pm_layerStack;
		static App* pm_Instance;

		bool pm_running = true;
		bool pm_minimized = false;
		bool pm_start_measure = false;
		float pm_dt =0.0f;

	};

	// client
	App* createApplication();

}
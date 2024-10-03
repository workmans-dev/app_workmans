#pragma once

#include "core.h"
#include "window.h"
#include "layerStack.h"
#include "events/event.h"
#include "events/applicationEvent.h"


namespace AppWMs {

	class APP_WMS_API App
	{
	public:
		App();
		~App();
		void onEvent(Event& e);
		void run();

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);

		inline Window& getWindow() { return *pm_window_ptr; }
		inline static App& getApp() { return *m_Instance; }
	private:
		bool onWindowClose(WindowCloseEvent& e);
		std::unique_ptr<Window> pm_window_ptr;
		bool pm_running = true;
		LayerStack m_layerStack;
		static App* m_Instance;
	};

	// client
	App* createApplication();

}
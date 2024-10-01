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
		bool onWindowClose(WindowCloseEvent& e);

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);
	private:
		std::unique_ptr<Window> pm_GLFWwindow_ptr;
		bool pm_running = true;
		LayerStack m_layerStack;
	};

	// client
	App* createApplication();

}
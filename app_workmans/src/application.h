#pragma once

#include "core.h"
#include "events/event.h"
#include "window.h"


namespace AppWMs {

	class APP_WMS_API App
	{
	public:
		App();
		~App();
		void onEvent(Event& e);
		void run();

	private:
		std::unique_ptr<Window> pm_GLFWwindow_ptr;
		bool pm_running = true;
	};
	// client
	App* createApplication();

}
#include "pch.h"
#include "core.h"
#include "application.h"

#include "events/applicationEvent.h"
#include "log.h"


namespace AppWMs {

	App::App() {
		std::cout << "App created" << std::endl;
	}
	App::~App() {
	}
	void App::run() {
		WindowResizeEvent e(1280, 720);
		if (e.isInCategory(EventCatergoryApplication)) {
			APP_WMS_CLI_TRACE(e);
		}
		if (e.isInCategory(EventCatergoryInput)) {
			APP_WMS_CLI_TRACE(e);
		}
		while (true);
	}

}
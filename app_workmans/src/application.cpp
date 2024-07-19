#include "pch.h"
#include "core.h"

#include "events/applicationEvent.h"
#include "log.h"
#include "application.h"

namespace AppWMs {

	App::App() {
	}
	App::~App() {
	}
	void App::run() {
		WindowResizeEvent e(1280, 720);
		//if (e.isInCategory(EventCatergoryApplication)) {
		//	APP_WMS_CLI_TRACE(e);
		//}
		//if (e.isInCategory(EventCatergoryInput)) {
		//	APP_WMS_CLI_TRACE(e);
		//}

	}

}
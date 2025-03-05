#pragma once
#include "core/core.h"

#ifdef APP_WMS_PLATFORM_WINDOWS
extern AppWMs::App* AppWMs::createApplication();

int main(int argc, char** argv) {
	AppWMs::Log::init();

	APP_WMS_PROFILE_BEGIN_SESSION("App start...", "AppWMs-profile_start.json");
	AppWMs::App* app = AppWMs::createApplication();
	APP_WMS_PROFILE_END_SESSION();

	APP_WMS_PROFILE_BEGIN_SESSION("App run...", "AppWMs-profile_run.json");
	app->run();
	APP_WMS_PROFILE_END_SESSION();

	APP_WMS_PROFILE_BEGIN_SESSION("App end...", "AppWMs-profile_end.json");
	delete app;
	APP_WMS_PROFILE_END_SESSION();
}
#endif
#pragma once
#ifdef APP_WMS_PLATFORM_WINDOWS
extern AppWMs::App* AppWMs::createApplication();

int main(int argc, char** argv) {
	AppWMs::Log::init();
	APP_WMS_CORE_TRACE("T E S T");
	int a = 5;
	APP_WMS_CLI_INFO("Hello Var={0}", 5);
	auto app = AppWMs::createApplication();
	app->run();
	delete app;
}
#endif
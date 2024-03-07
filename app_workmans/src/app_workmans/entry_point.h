#pragma once
#ifdef APP_WMS_PLATFORM_WINDOWS
extern AppWMs::App* AppWMs::createApplication();

int main(int argc, char** argv) {
	auto app = AppWMs::createApplication();
	app->run();
	delete app;
}
#endif
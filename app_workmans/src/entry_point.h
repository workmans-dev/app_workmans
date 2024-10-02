#pragma once
#ifdef APP_WMS_PLATFORM_WINDOWS
extern AppWMs::App* AppWMs::createApplication();

int main(int argc, char** argv) {
	AppWMs::Log::init();
	std::cout << "ready to read : createApplication" << std::endl;
	auto app = AppWMs::createApplication();

	app->run();
	delete app;
}
#endif
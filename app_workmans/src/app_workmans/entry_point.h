#pragma once
#ifdef APP_WMS_PLATFORM_WINDOWS

extern APP_WMS::Application* APP_WMS::createApplication();

int main(int argc, char** argv) {
	auto app = APP_WMS::createApplication();
	app->run();
	delete app;
}

#endif
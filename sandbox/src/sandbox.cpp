#include <app_workmans.h>

class Sandbox:public APP_WMS::Application {
	public:
		Sandbox() {};
		~Sandbox() {};
};

APP_WMS::Application* APP_WMS::createApplication() {
	return new Sandbox();
}
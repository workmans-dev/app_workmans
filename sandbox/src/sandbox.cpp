#include <app_workmans.h>

class Sandbox:public APP_WMS::Application {
	public:
		Sandbox() {};
		~Sandbox() {};
};

int main() {
	Sandbox* sandbox = new Sandbox();
	sandbox->run();
	delete sandbox;
} 
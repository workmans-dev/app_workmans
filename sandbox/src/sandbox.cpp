#include <app_workmans.h>

class Sandbox : public AppWMs::App
{
public:
	Sandbox() {
	}
	~Sandbox() {
	}
};

AppWMs::App* AppWMs::createApplication(){
	return new Sandbox();
}
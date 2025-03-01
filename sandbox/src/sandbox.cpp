#include "core/app_workmans.h"
#include "core/entry_point.h"

#include "Viewport.h"





class Sandbox : public AppWMs::App
{
public:
	Sandbox() {
		pushLayer(new AppWMs::Viewport());
	}
	~Sandbox() {

	}

};

AppWMs::App* AppWMs::createApplication(){
	return new Sandbox();
}
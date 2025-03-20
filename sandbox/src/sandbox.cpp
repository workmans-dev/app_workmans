#include "core/app_workmans.h"
#include "core/entry_point.h"

#include "mainWindow.h"




class Sandbox : public AppWMs::App
{
public:
	Sandbox() {
		// Viewport class contains all the components of the scene.
		// SceneProperties, Scene, Entity
		pushLayer(new AppWMs::MainWindow());
	}
	~Sandbox() {

	}

};

AppWMs::App* AppWMs::createApplication(){
	return new Sandbox();
}
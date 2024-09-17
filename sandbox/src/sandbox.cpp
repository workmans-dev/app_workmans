#include "app_workmans.h"
#include <iostream>
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
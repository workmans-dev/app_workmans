#include "app_workmans.h"
#include <iostream>
class Sandbox : public AppWMs::App
{
public:
	Sandbox() {
		std::cout << "SandBox created" << std::endl;
	}
	~Sandbox() {
	}
};

AppWMs::App* AppWMs::createApplication(){
	return new Sandbox();
}
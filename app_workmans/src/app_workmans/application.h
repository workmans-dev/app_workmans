#pragma once
#include "core.h"
#include "stdio.h"

namespace AppWMs {

	class APP_WMS_API App
	{
	public:
		App();
		~App();
		void run();
	};
	// client
	App* createApplication();

}
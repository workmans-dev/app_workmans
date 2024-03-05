#pragma once
#include "core.h"

namespace APP_WMS {
	class APP_WMS_API Application{
		public:
			Application();
			virtual ~Application();
			void run();
	};
}
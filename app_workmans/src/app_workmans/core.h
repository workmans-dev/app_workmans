#pragma once

#ifdef APP_WMS_PLATFORM_WINDOWS
	#ifdef APP_WMS_BUILD_DLL
		#define APP_WMS_API __declspec(dllexport)
	#else
		#define APP_WMS_API __declspec(dllimport)
	#endif
#else
	#error ONLY SUPPORT WINDOWS!
#endif
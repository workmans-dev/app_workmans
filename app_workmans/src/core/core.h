#pragma once
#include <memory>

#ifdef APP_WMS_PLATFORM_WINDOWS
#if APP_WMS_DY_LINK
	#ifdef APP_WMS_BUILD_DLL
		#define APP_WMS_API __declspec(dllexport)
	#else
		#define APP_WMS_API __declspec(dllimport)
	#endif
#else
#define APP_WMS_API
#endif
#else
	#error ONLY SUPPORT WINDOWS!
#endif

#ifdef APP_WMS_DEBUG
#define APP_WMS_ENABLE_ASSERTS
#endif


#ifdef APP_WMS_ENABLE_ASSERTS
	#define APP_WMS_CLI_ASSERTS(x,...){if(!(x)){APP_WMS_CLI_ERROR("Assertion Failed : {0}", __VA_ARGS__); __debugbreak();}}
	#define APP_WMS_CORE_ASSERTS(x,...){if(!(x)){APP_WMS_CORE_ERROR("Assertion Failed : {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define APP_WMS_CLI_ASSERTS(x,...)
	#define APP_WMS_CORE_ASSERTS(x,...)
#endif

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) [this](auto&&... args)->decltype(auto){ return this->fn(std::forward<decltype(args)>(args)...); }

namespace AppWMs {
	template <typename T>
	using Scope = std::unique_ptr<T>;
	template <typename T>
	using Ref = std::shared_ptr<T>;
}
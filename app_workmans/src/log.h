#pragma once

#include "core.h"
#include "spdlog/spdlog.h"

namespace AppWMs {
	class APP_WMS_API Log {
	public:
		static void init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger();
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger();
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define APP_WMS_CORE_TRACE(...) ::AppWMs::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define APP_WMS_CORE_ERROR(...) ::AppWMs::Log::GetCoreLogger()->error(__VA_ARGS__);
#define APP_WMS_CORE_WARN(...) ::AppWMs::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define APP_WMS_CORE_INFO(...) ::AppWMs::Log::GetCoreLogger()->info(__VA_ARGS__);
#define APP_WMS_CORE_FATAL(...) ::AppWMs::Log::GetCoreLogger()->fatal(__VA_ARGS__);

#define APP_WMS_CLI_TRACE(...) ::AppWMs::Log::GetClientLogger()->trace(__VA_ARGS__);
#define APP_WMS_CLI_ERROR(...) ::AppWMs::Log::GetClientLogger()->error(__VA_ARGS__);
#define APP_WMS_CLI_WARN(...) ::AppWMs::Log::GetClientLogger()->warn(__VA_ARGS__);
#define APP_WMS_CLI_INFO(...) ::AppWMs::Log::GetClientLogger()->info(__VA_ARGS__);
#define APP_WMS_CLI_FATAL(...) ::AppWMs::Log::GetClientLogger()->fatal(__VA_ARGS__);
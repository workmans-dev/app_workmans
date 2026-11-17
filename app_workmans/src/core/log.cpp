#include "pch.h"
#include "core/log.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace AppWMs {
	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;

	Ref<spdlog::logger>& Log::GetCoreLogger() {
		return s_CoreLogger;
	}
	Ref<spdlog::logger>& Log::GetClientLogger() {
		return s_ClientLogger;
	}

	void Log::init() {
#ifdef APP_WMS_PLATFORM_WINDOWS
		// The sources compile with /utf-8, so log strings go out as UTF-8 bytes, while the console
		// interprets them in the system output code page (949 in a Korean environment) and garbles them.
		// This sets the console side to UTF-8. Redirecting to a file was never a problem
		// (there the reader only needs to decode as UTF-8).
		SetConsoleOutputCP(CP_UTF8);
#endif
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("WORKMANs");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}
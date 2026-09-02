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
		// 소스는 /utf-8로 컴파일되어 로그 문자열이 UTF-8 바이트로 나가는데,
		// 콘솔은 시스템 출력 코드페이지(한국어 환경 949)로 해석해서 한글이 깨진다.
		// 콘솔 쪽을 UTF-8로 맞춘다. 파일로 리다이렉트할 때는 원래 문제가 없다
		// (그쪽은 읽는 쪽이 UTF-8로 디코딩하면 된다).
		SetConsoleOutputCP(CP_UTF8);
#endif
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("WORKMANs");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}
#pragma once
#include "core/timer.h"
#include <thread>
#include <string>
#include <fstream>
#include <chrono> 
//#define SCOPE_PROFILE_TIMER(name) TimerProfile timer##__LINE__(name, [&](ProfileData profile_data) {pm_profile_data_vec.push_back(profile_data);})


#define APP_WMS_PROFILE 1
#if APP_WMS_PROFILE
	#define APP_WMS_PROFILE_BEGIN_SESSION(name, filePath) ::AppWMs::Instrumentation::get_instance().beginSession(name, filePath)
	#define APP_WMS_PROFILE_END_SESSION() ::AppWMs::Instrumentation::get_instance().endSession()
	#define SCOPE_PROFILE_TIMER(name) ::AppWMs::TimerProfile timer##__LINE__(name);
	#define APP_WMS_PROFILE_FUNC() SCOPE_PROFILE_TIMER(__FUNCSIG__)
#else
	#define APP_WMS_PROFILE_BEGIN_SESSION(name, filePath)
	#define APP_WMS_PROFILE_END_SESSION()
	#define SCOPE_PROFILE_TIMER(name)
	#define APP_WMS_PROFILE_FUNC()
#endif




namespace AppWMs {

	struct ProfileData {
		std::string name;
		long long start, end;
		size_t threadID;
	};

	struct InstrumentationSession {
		std::string name;
	};

	class Instrumentation {
	public:
		Instrumentation() :pm_seesion_ptr(nullptr), pm_profileCount(0) {

		}
		void beginSession(const std::string& name,const std::string& filePath = "results.json") {
			pm_outputStream.open(filePath);
			writeHeader();
			pm_seesion_ptr = new InstrumentationSession{ name };
		}
		void endSession() {
			writeFooter();
			pm_outputStream.close();
			delete pm_seesion_ptr;
			pm_profileCount = 0;
		}
		void writeHeader();
		void writeFooter();
		void writeProfile(const ProfileData& profile_data);

		static Instrumentation& get_instance() {
			static Instrumentation instance;
			return instance;
		}

	private:
		InstrumentationSession* pm_seesion_ptr;
		int pm_profileCount;
		std::ofstream pm_outputStream;
	};


	class TimerProfile {
	public:
		TimerProfile(const char* name) :pm_name(name), pm_stopped(false) {
			pm_startTime = std::chrono::high_resolution_clock::now();
		}

		~TimerProfile() {
			if (!pm_stopped)
				stop();
		}

		void stop() {
			auto endTime = std::chrono::high_resolution_clock::now();
			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(pm_startTime).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

			size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
			Instrumentation::get_instance().writeProfile({ pm_name, start, end, threadID });
			pm_stopped = true;

		}

	private:
		const char* pm_name;
		bool pm_stopped;
		std::chrono::time_point<std::chrono::steady_clock> pm_startTime;
	};
}
#include "pch.h"
#include "debugs/instrumentation.h"

namespace AppWMs{
	void Instrumentation::writeHeader()
	{
		pm_outputStream << "{\"otherData:{}\", \"traceEvents\":[";
		pm_outputStream.flush();
	}

	void Instrumentation::writeFooter()
	{
		pm_outputStream << "]}";
		pm_outputStream.flush();
	}
	void Instrumentation::writeProfile(const ProfileData& profile_data)
	{
		if (pm_profileCount++ > 0)
			pm_outputStream << ",\n";
		std::string str_name = profile_data.name;
		std::replace(str_name.begin(), str_name.end(), '"', '\'');

		pm_outputStream << "{";
		pm_outputStream << "\"cat\":\"function\",";
		pm_outputStream << "\"dur\":" << (profile_data.end- profile_data.start) << ',';
		pm_outputStream << "\"name\":\"" << str_name << "\",";
		pm_outputStream << "\"ph\":\"X\",";
		pm_outputStream << "\"pid\":0,";
		pm_outputStream << "\"tid\":" << profile_data.threadID << ",";
		pm_outputStream << "\"ts\":" << profile_data.start;
		pm_outputStream << "}";

		pm_outputStream.flush();
	}
}

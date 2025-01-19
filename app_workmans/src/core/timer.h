#pragma once

namespace AppWMs {
	class Timer {
	public:
		Timer(float time=0.0f) :pm_time(time) {

		}
		float getTime_sec() const{
			return pm_time;
		}
		float getTime_millisec() const{
			return pm_time * 1000.0f;
		}

		operator float() { return getTime_sec(); }
		

	private:
		float pm_time;
	};
	
}
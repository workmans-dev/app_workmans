#pragma once
#include "event.h"

#include <sstream>

namespace AppWMs {
	class APP_WMS_API WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int w, unsigned int h):pm_width(w), pm_height(h){}
		inline unsigned int getWidth() const { return pm_width; }
		inline unsigned int getHeight() const { return pm_height; }
		std::string toString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent : " << pm_width << " , " << pm_height;
			return ss.str();
		}
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCatergoryApplication)
	private:
		unsigned int pm_width;
		unsigned int pm_height;

	};


	class APP_WMs_API WindowCloseEvent : public Event {
	public:
		WindowCloseEvent(){}
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCatergoryApplication)
	};


	class APP_WMs_API AppTick : public Event {
	public:
		AppTick() {}
		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCatergoryApplication)
	};


	class APP_WMs_API AppUpdate : public Event {
	public:
		AppUpdate() {}
		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCatergoryApplication)
	};


	class APP_WMs_API AppRender : public Event {
	public:
		AppRender() {}
		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCatergoryApplication)
	};


	class APP_WMs_API WindowMoved : public Event {
	public:
		WindowMoved() {}
		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCatergoryApplication)
	};

}
#pragma once
#include "events/event.h"


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
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int pm_width, pm_height;

	};


	class APP_WMS_API WindowCloseEvent : public Event {
	public:
		WindowCloseEvent(){}
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};


	class APP_WMS_API AppTickEvent : public Event {
	public:
		AppTickEvent() {}
		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};


	class APP_WMS_API AppUpdateEvent : public Event {
	public:
		AppUpdateEvent() {}
		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};


	class APP_WMS_API AppRenderEvent : public Event {
	public:
		AppRenderEvent() {}
		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};


	class APP_WMS_API WindowMovedEvent : public Event {
	public:
		WindowMovedEvent() {}
		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

}
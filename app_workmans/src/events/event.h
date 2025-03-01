#pragma once
#include "pch.h"
#include "core/core.h"

namespace AppWMs {
	enum class EventType {
		None=0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender, 
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory {
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

	#define EVENT_CLASS_TYPE(type) static EventType getStaticType() {return EventType::##type;}\
									virtual EventType getEventType() const override {return getStaticType();}\
									virtual const char* getName() const override {return #type;}
	//// category is the EventCategory, return EventCategory::EventCategory
	#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override {return category;}


	class  Event {
		friend class EventDispatcher;
	public:
		virtual ~Event() = default;
		bool p_handled = false;

		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getCategoryFlags() const = 0;
		virtual std::string toString() const { return getName(); }
		bool isInCategory(EventCategory category) {return getCategoryFlags() & category;}
	};


	class EventDispatcher {
	public:
		// pm_event(event) is a Event object , type : <EventType> //
		EventDispatcher(Event& event):pm_event(event){}

		template <typename T, typename F>
		bool Dispatch(const F& func) {
			if (pm_event.getEventType()==T::getStaticType()) {
				pm_event.p_handled = func(static_cast<T&>(pm_event));
				return true;
			}
			return false;
		}
	private:
		Event& pm_event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.toString();
	}
}


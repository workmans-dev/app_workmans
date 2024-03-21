#pragma once
#include "core.h"

#include <string>
#include <functional>

namespace AppWMs {
	enum class EventType {
		None=0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender, 
		KeyPressed, KeyReleased, 
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory {
		None = 0,
		EventCatergoryApplication = BIT(0),
		EventCatergoryInput = BIT(1),
		EventCatergoryKeyboard = BIT(2),
		EventCatergoryMouse = BIT(3),
		EventCatergoryMouseButton = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() {return EventType::##type;}\
								virtual EventType getEventType() const override {return getStaticType();}\
								virtual const char* getName() const override {return #type;}

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override {return category;}

	class APP_WMS_API Event {
		friend class EventDispatcher;
	public:
		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getCategoryFlags() const = 0;
		virtual std::string ToString() const { return getName(); }

		inline bool isInCategory(EventCategory category) {return getCategoryFlags() & category;}
	protected:
		bool m_handled = false;
	};

	class EventDispatcher {
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event):m_event(event){}
		template <typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_event.getEventType()==T::GetStatticType()) {
				m_event.m_handled = func(*(T*)&m_event);
				return true;
			}
			return false;
		}
	private:
		Event& m_event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}
}


#pragma once
#include "events/event.h"

namespace AppWMs {
	class APP_WMS_API KeyEvent:public Event {
	public:
		inline int getKeyCode() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(EventCatergoryKeyboard | EventCatergoryInput)
	protected:
		KeyEvent(int keycode) :m_KeyCode(keycode){}
		int m_KeyCode;
	};

	class APP_WMS_API KeyPressedEvent :public KeyEvent {
	public:
		KeyPressedEvent(int keycode, int repeatCount) 
			:KeyEvent(keycode), pm_repeatCount(repeatCount){}
		inline int getRepeatCount() const { return pm_repeatCount; }
		std::string toString() {
			std::stringstream ss;
			ss << "KeyPressed : " << m_KeyCode << "Repeated time : " << pm_repeatCount;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int pm_repeatCount;
	};

	class APP_WMS_API KeyReleasedEvent :public KeyEvent {
	public:
		KeyReleasedEvent(int keycode):KeyEvent(keycode) {}
		std::string toString() {
			std::stringstream ss;
			ss << "KeyReleased : " << m_KeyCode ;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	};


	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int keycode)
			: KeyEvent(keycode) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
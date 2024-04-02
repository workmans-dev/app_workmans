#pragma once
#include "event.h"

namespace AppWMs {
	class APP_WMS_API KeyEvent:public Event {
	public:
		inline int getKeyCode() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(EventCatergoryKeyboard | EventCatergoryInput)
	protected:
		KeyEvent(int keycode) :m_KeyCode(keycode){}
		int m_KeyCode;
	};
}
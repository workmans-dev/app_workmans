#pragma once
#include "events/event.h"

namespace AppWMs {
	class  MouseMovedEvent :public Event {
		public:
			MouseMovedEvent(float x, float y):m_xPnt(x), m_yPnt(y){}
			inline float get_xPnt() const { return m_xPnt; }
			inline float get_yPnt() const { return m_yPnt; }
			std::string toString() const override{
				std::stringstream ss;
				ss << "mouse pnt : [" << m_xPnt << ", " << m_yPnt << "]";
				return ss.str();
			}
			//// ----- to declare which event is ocurring.
			EVENT_CLASS_TYPE(MouseMoved);
			EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse);
			//// ----- end
		private:
			float m_xPnt, m_yPnt;
	};

	class  MouseScrollEvent :public Event {
		public:
			MouseScrollEvent(float x, float y) :m_xScroll(x), m_yScroll(y) {}
			inline float get_xScroll() const { return m_xScroll; }
			inline float get_yScroll() const { return m_yScroll; }
			std::string toString() const override {
				std::stringstream ss;
				ss << "mouse scolled : [" << m_xScroll << ", " << m_yScroll << "]";
				return ss.str();
			}
			//// ----- to declare which event is ocurring.
			EVENT_CLASS_TYPE(MouseScrolled);
			EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse);
			//// ----- end
		private:
			float m_xScroll, m_yScroll=1.0f;
	};

	class  MouseButtonEvent :public Event {
		public:
			inline int get_mouseButton() const { return m_mouseBtn; }
			//// ----- to declare which event is ocurring.
			EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse);
			//// ----- end
		protected:
			MouseButtonEvent(int mouseBtn) :m_mouseBtn(mouseBtn) {}
			int m_mouseBtn;
	};

	class  MousePressedEvent:public MouseButtonEvent
	{
	public:
		MousePressedEvent(int btn) :MouseButtonEvent(btn){}
		std::string toString() const override {
			std::stringstream ss;
			ss << "mouse pressed : " << m_mouseBtn;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButtonPressed);
	};

	class  MouseReleasedEvent :public MouseButtonEvent 
	{
	public:
		MouseReleasedEvent(int btn) :MouseButtonEvent(btn) {}
		std::string toString() const override {
			std::stringstream ss;
			ss << "mouse released : " << m_mouseBtn;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MouseButtonReleased);
	};
}
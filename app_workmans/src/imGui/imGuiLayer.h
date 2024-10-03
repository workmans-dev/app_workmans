#pragma once
#include "layer.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "events/applicationEvent.h"


namespace AppWMs {
	class APP_WMS_API ImGuiLayer :public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();
		void onAttach();
		void onDetach();
		void onUpdate();
		void onEvent(Event& event);
	private:
		float m_time = 0.0f;

	private:
		// event system 
		bool onMousePressedEvent(MousePressedEvent& e);
		bool onMouseReleasedEvent(MouseReleasedEvent& e);
		bool onMouseMovedEvent(MouseMovedEvent& e);
		bool onMouseScrollEvent(MouseScrollEvent& e);
		bool onKeyReleasedEvent(KeyReleasedEvent& e);
		bool onKeyPressedEvent(KeyPressedEvent& e);
		bool onKeyTypedEvent(KeyTypedEvent& e);
		bool onWindowResizeEvent(WindowResizeEvent& e);
	};

}

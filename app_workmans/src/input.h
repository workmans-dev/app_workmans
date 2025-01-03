#pragma once
#include "core.h"

namespace AppWMs {
	class APP_WMS_API Input {
	public:
		inline static bool IsKeyPressed(int keycode) { return m_Input_instance_ptr->IsKeyPressedImpl(keycode); }

		inline static bool IsMousePressed(int mouseButton) { return m_Input_instance_ptr->IsMousePressedImpl(mouseButton); }
		inline static std::pair<float, float> getMousePos() { return m_Input_instance_ptr->getMousePosImpl(); }
		inline static float getMouseX() { return m_Input_instance_ptr->getMouseXImpl(); }
		inline static float getMouseY() { return m_Input_instance_ptr->getMouseYImpl(); }


	private:
		static Input* m_Input_instance_ptr;
	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMousePressedImpl(int mouseButton) = 0;
		virtual std::pair<float, float> getMousePosImpl() = 0;
		virtual float getMouseXImpl() = 0;
		virtual float getMouseYImpl() = 0;
	};
}
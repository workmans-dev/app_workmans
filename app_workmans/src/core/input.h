#pragma once
#include "core/core.h"

namespace AppWMs {
	class  Input {
	public:
		virtual ~Input() = default;
		inline static bool IsKeyPressed(int keycode) { return m_Input_instance_ptr->IsKeyPressedImpl(keycode); }

		inline static bool IsMousePressed(int mouseButton) { return m_Input_instance_ptr->IsMousePressedImpl(mouseButton); }
		inline static std::pair<float, float> getMousePos() { return m_Input_instance_ptr->getMousePosImpl(); }
		inline static float getMouseX() { return m_Input_instance_ptr->getMouseXImpl(); }
		inline static float getMouseY() { return m_Input_instance_ptr->getMouseYImpl(); }
		inline static float getMouseDistX() { return m_Input_instance_ptr->getMouseDistXImpl(); }
		inline static float getMouseDistY() { return m_Input_instance_ptr->getMouseDistYImpl(); }
		
		inline static std::pair<float, float> getDist_mouseMoved()
		{
			return m_Input_instance_ptr->getDist_mouseMovedImpl();
		}

	private:
		static Input* m_Input_instance_ptr;

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMousePressedImpl(int mouseButton) = 0;
		virtual std::pair<float, float> getDist_mouseMovedImpl() = 0;


		virtual std::pair<float, float> getMousePosImpl() = 0;

		virtual float getMouseXImpl() = 0;
		virtual float getMouseYImpl() = 0;

		virtual float getMouseDistXImpl() = 0;
		virtual float getMouseDistYImpl() = 0;
	};
}
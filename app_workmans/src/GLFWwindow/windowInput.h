#pragma once
#include "core/input.h"

namespace AppWMs {
	class WindowInput : public Input {
	public:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMousePressedImpl(int mouseButton) override;
		virtual std::pair<float, float> getMousePosImpl() override;
		virtual float getMouseXImpl() override;
		virtual float getMouseYImpl() override;
		virtual float getMouseDistXImpl() override;
		virtual float getMouseDistYImpl() override;
		virtual std::pair<float, float> getDist_mouseMovedImpl() override;
	protected:
		float first_pressed_x = 0.0f;
		float first_pressed_y = 0.0f;
		bool capture = false;


	};
}
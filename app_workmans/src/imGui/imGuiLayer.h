#pragma once
#include "layer.h"

namespace AppWMs {
	class APP_WMS_API ImGuiLayer :public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();
		virtual void onAttach();
		virtual void onDetach();
		virtual void onUpdate();
		virtual void onEvent(Event& e);
	private:
		float m_time = 0.0f;
	};

}

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
		virtual void onAttach() override;
		virtual void onDetach() override;
		virtual void onImGuiRender() override;

		void begin();
		void end();
	private:
		float m_time = 0.0f;


	};

}

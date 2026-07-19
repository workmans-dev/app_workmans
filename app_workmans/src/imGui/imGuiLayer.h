#pragma once
#include "core/layer.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "events/applicationEvent.h"
#include "GLFWwindow/win_window.h"

namespace AppWMs {
	class ImGuiLayer :public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();
		virtual void onAttach() override;
		virtual void onEvent(Event& event) override;		
		virtual void onDetach() override;

		void InitVulkanBackend();
		void blockEvent(bool fact) { pm_eventBlocking = fact; };
		void begin();
		void end();
	private:
		float m_time = 0.0f;
		bool pm_eventBlocking = false;

	};

}

#pragma once
#include "core.h"
#include "events/event.h"

namespace AppWMs{
	class APP_WMS_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();
		virtual void onUpdate() {}
		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onEvent(Event& event) {}

		inline const std::string& getName() const { return m_debugName;}

	protected:
		std::string m_debugName;
	};
}
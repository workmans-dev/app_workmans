#pragma once
#include "core/core.h"
#include "core/timer.h"

#include "events/event.h"

namespace AppWMs{
	class Layer {
		public:
			Layer(const std::string& name);
			virtual ~Layer();

			virtual void onAttach() {}
			virtual void onDetach() {}
			virtual void onUpdate(Timer timer) { }
			virtual void onEvent(Event& event) {}

			virtual void onImGuiRender() {}

			inline const std::string& getName() const { return m_debugName;}

		protected:
			std::string m_debugName;
	};
}
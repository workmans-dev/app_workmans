#pragma once
#include "core/core.h"
#include "layer.h"

namespace AppWMs {
	class LayerStack {
	public:
		LayerStack();
		~LayerStack();
		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);
		void popLayer(Layer* layer);
		void popOverlay(Layer* overlay);
		// Calls onDetach and then destroys everything. Exposed so App can control the shutdown order.
		//    Safe to call twice (the destructor uses it too).
		void clear();

		std::vector<Layer*>::iterator begin() 
		{ return m_layers.begin(); }
		std::vector<Layer*>::iterator end() 
		{ return m_layers.end(); }

	private:
		std::vector<AppWMs::Layer*> m_layers;
		unsigned int m_layer_insert_index = 0;
	};
}
#include "pch.h"

#include "layerStack.h"


namespace AppWMs {
	LayerStack::LayerStack() {
		m_layers_iter = m_layers.begin();
	}
	LayerStack::~LayerStack() {
		for (Layer* layer : m_layers) {
			delete layer;
		}
	}
	void LayerStack::pushLayer(Layer* layer) {
		m_layers_iter = m_layers.emplace(m_layers_iter, layer);
		//m_layers_iter++;
	}
	void LayerStack::pushOverlay(Layer* overlay) {
		m_layers.emplace_back(overlay);
	}
	void LayerStack::popLayer(Layer* layer) {
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end()) {
			//layer->onDetach();
			m_layers.erase(it);
			m_layers_iter--;
		}
	}
	void LayerStack::popOverlay(Layer* overlay) {
		auto it = std::find(m_layers.begin(), m_layers.end(), overlay);
		if (it != m_layers.end()) {
			//overlay->onDetach();
			m_layers.erase(it);
		}
	}
}
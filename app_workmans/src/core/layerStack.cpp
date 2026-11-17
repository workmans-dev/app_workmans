#include "pch.h"

#include "layerStack.h"


namespace AppWMs {
	LayerStack::LayerStack() {
	}

	LayerStack::~LayerStack() {
		clear();
	}

	void LayerStack::clear() {
		// WARNING: the vector order is the destruction dependency order. Do not reverse it.
		//    It is [MainWindow, ImGuiLayer], so the RenderTarget is destroyed before ImGui's Vulkan backend.
		//    ~VulkanRenderTarget calls ImGui_ImplVulkan_RemoveTexture, so it has to be this order.
		for (Layer* layer : m_layers) {
			layer->onDetach();   // this used not to be called, so ImGui_ImplVulkan_Shutdown never ran
			delete layer;
		}
		m_layers.clear();
		m_layer_insert_index = 0;
	}

	void LayerStack::pushLayer(Layer* layer) {
		m_layers.emplace(m_layers.begin()+ m_layer_insert_index, layer);
		m_layer_insert_index++;
	}

	void LayerStack::pushOverlay(Layer* overlay) {
		m_layers.emplace_back(overlay);
	}

	void LayerStack::popLayer(Layer* layer) {
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end()) {
			//layer->onDetach();
			m_layers.erase(it);
			m_layer_insert_index--;
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
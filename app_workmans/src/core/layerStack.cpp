#include "pch.h"

#include "layerStack.h"


namespace AppWMs {
	LayerStack::LayerStack() {
	}

	LayerStack::~LayerStack() {
		clear();
	}

	void LayerStack::clear() {
		// ⚠️ 벡터 순서가 곧 파괴 의존 순서다. 뒤집으면 안 된다.
		//    [MainWindow, ImGuiLayer] 순이라 FrameBuffer가 ImGui 벌칸 백엔드보다 먼저 파괴된다.
		//    ~VulkanRenderTarget이 ImGui_ImplVulkan_RemoveTexture를 부르므로 이 순서여야 한다.
		for (Layer* layer : m_layers) {
			layer->onDetach();   // 예전엔 호출되지 않아 ImGui_ImplVulkan_Shutdown이 실행 안 됐다
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
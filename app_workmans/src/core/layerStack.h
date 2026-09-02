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
		// 🟢 onDetach 호출 후 전부 파괴. 종료 순서를 App이 직접 잡기 위해 노출한다.
		//    두 번 불러도 안전하다(소멸자도 이걸 쓴다).
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
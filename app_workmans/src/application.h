#pragma once

#include "core.h"
#include "window.h"
#include "layerStack.h"
#include "events/event.h"
#include "events/applicationEvent.h"
#include "imGui/imGuiLayer.h"
#include "renderer/shader.h"
#include "renderer/buffer.h"
#include "renderer/vertexArray.h"

namespace AppWMs {

	class  App
	{
	public:
		App();
		~App();
		void onEvent(Event& e);
		void run();

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);

		inline Window& getWindow() { return *pm_window_ptr; }
		inline static App& getApp() { return *pm_Instance; }
	private:
		bool onWindowClose(WindowCloseEvent& e);
		std::unique_ptr<Window> pm_window_ptr;

		std::unique_ptr<Shader> pm_shader_ptr;
		std::shared_ptr<VertexArray> pm_vertexArray_ptr;


		ImGuiLayer* pm_imGuiLayer_ptr;

		bool pm_running = true;
		LayerStack pm_layerStack;
		static App* pm_Instance;

	};

	// client
	App* createApplication();

}
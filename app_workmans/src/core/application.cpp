#include "pch.h"
#include "application.h"
#include "renderer/renderer.h"
#include "core/timer.h"
#include <glfw/glfw3.h>

namespace AppWMs {
	App* App::pm_Instance = nullptr;

	App::App() {
		// 1. 창문 만들기 (Vulkan Instance, Device 등 기본 뼈대 생성)
		pm_window_ptr = std::unique_ptr<Window>(Window::create());
		pm_window_ptr->setData_EventCallBack(BIND_EVENT_FN(App::onEvent));

		// 2. 렌더러 초기화 (여기서 벌칸 RenderPass, DescriptorPool 등 핵심 부품 완성!)
		Renderer::init();

		// 3. 💥 드디어 임구이 레이어 오브젝트 생성! (여기서 오브젝트가 메모리에 올라갑니다)
		pm_imGuiLayer_ptr = new ImGuiLayer(); // 내부 onAttach() 실행되어 순수 ImGui 컨텍스트 생성
		pushOverlay(pm_imGuiLayer_ptr);

		// 4. 🔥 오브젝트가 확실히 존재하므로, 이제 메서드를 호출해서 벌칸 부품을 주입합니다!
		auto winPtr = static_cast<Win_Window*>(pm_window_ptr.get());
		pm_imGuiLayer_ptr->InitVulkanBackend(winPtr->m_data);
	}


	App::~App() {

		Renderer::shutdown();
	}

	// send all events of layer events to Win_window : pm_window_ptr
	void App::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(App::onWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(App::onWindowResize));
		for (auto it = pm_layerStack.begin(); it != pm_layerStack.end();++it)
		{
			(*it)->onEvent(event);
			if (event.p_handled)
				break;
		}
	}

	void App::run() {
		while (pm_running) 
		{
			float time = (float)glfwGetTime();
			Timer timer = time - pm_dt;
			pm_dt = timer;

			if (!pm_minimized) {
				// layer base class onUpdate
				for (Layer* layer : pm_layerStack) 
					layer->onUpdate(timer);
			}

			// gui layer
			pm_imGuiLayer_ptr->begin();
			for (Layer* layer : pm_layerStack)
			{
				// each layer(ui component) has to 
				// have this method if gui feature needs !
				layer->onImGuiRender();
			}
			pm_imGuiLayer_ptr->end();

			// final update for application window
			pm_window_ptr->onUpdate();


		}
	}

	bool App::onWindowClose(WindowCloseEvent& e) {
		pm_running = false;
		return true;
	}

	bool App::onWindowResize(WindowResizeEvent& e)
	{
		uint32_t w = e.getWidth();
		uint32_t h = e.getHeight();
		if (w==0 || h==0) {
			pm_minimized = true;
		}
		pm_minimized = false;
		Renderer::onWindowResize(w, h);
		return false;
	}

	bool App::onWindowCameraAim(MousePressedEvent& mouse_event, KeyPressedEvent& key_event)
	{
		if (mouse_event.get_mouseButton() == GLFW_MOUSE_BUTTON_LEFT &&
			key_event.getKeyCode()==GLFW_KEY_LEFT_ALT) {
			pm_start_measure = true;
			
		}
		pm_start_measure = false;
		return false;
	}

	void App::pushLayer(Layer* layer) {
		pm_layerStack.pushLayer(layer);
		layer->onAttach();
	}

	void App::pushOverlay(Layer* overlay) {
		pm_layerStack.pushOverlay(overlay);
		overlay->onAttach();
	}
	void App::close() {
		pm_running = false;
	}
}
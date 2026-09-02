#include "pch.h"
#include "application.h"
#include "renderer/renderer.h"
#include "core/timer.h"
#include <glfw/glfw3.h>

namespace AppWMs {
	App* App::pm_Instance = nullptr;

	App::App() {

		APP_WMS_CORE_ASSERTS(!pm_Instance, "App already exists!");
		pm_Instance = this;
		// 1. 창문 만들기 (Vulkan Instance, Device 등 기본 뼈대 생성)
		pm_window_ptr = std::unique_ptr<Window>(Window::create());
		pm_window_ptr->setData_EventCallBack(BIND_EVENT_FN(App::onEvent));

		// 2. 렌더러 초기화 (여기서 벌칸 RenderPass, DescriptorPool 등 핵심 부품 완성!)
		Renderer::init();

		// 3. 💥 드디어 임구이 레이어 오브젝트 생성! (여기서 오브젝트가 메모리에 올라갑니다)
		pm_imGuiLayer_ptr = new ImGuiLayer(); // 내부 onAttach() 실행되어 순수 ImGui 컨텍스트 생성
		pushOverlay(pm_imGuiLayer_ptr);

		// 4. 벌칸 부품을 주입합니다!
		pm_imGuiLayer_ptr->InitVulkanBackend(
			static_cast<GLFWwindow*>(pm_window_ptr->getGLFWWindow()));
	}


	App::~App() {
		// ⚠️ 종료는 순서가 전부다. 소멸자 기본 순서에 맡기면 정적 수명인
		//    RendererAPI가 디바이스보다 늦게 파괴돼 use-after-free가 난다.
		//    그래서 여기서 명시적으로 몰아준다.
		//
		//  1. GPU가 마지막 프레임을 끝낼 때까지 대기
		//     (close 이벤트가 온 프레임도 끝까지 submit되므로 반드시 필요)
		RendererCommand::waitIdle();

		//  2. 레이어 파괴 — MainWindow(FrameBuffer/파이프라인) 다음 ImGuiLayer(벌칸 백엔드).
		//     FrameBuffer 소멸자가 ImGui_ImplVulkan_RemoveTexture를 부르므로 이 순서여야 한다.
		pm_layerStack.clear();

		//  3. RendererAPI 리소스 — 디스크립터풀은 ImGui가 쓰던 것이라 2번 뒤여야 한다
		Renderer::shutdown();

		//  4. VulkanContext(device/surface/instance)는 pm_window_ptr 소멸자가 파괴한다.
		//     멤버 소멸은 이 본문이 끝난 뒤이므로 자연히 마지막이 된다.
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

			// 🟢 이벤트 폴링은 프레임 최상단 (입력 지연 최소화)
			pm_window_ptr->onUpdate();

			// 🟢 프레임 개시: acquire + 커맨드버퍼 begin.
			//    실패(minimized/out-of-date)면 이번 프레임 전체 스킵.
			if (pm_minimized || !Renderer::beginFrame()) {
				continue;
			}

			// 오프스크린 렌더 (각 레이어가 자기 FrameBuffer에 3D 씬 기록)
			for (Layer* layer : pm_layerStack)
				layer->onUpdate(timer);

			// ImGui (스왑체인 렌더패스에 UI 기록)
			pm_imGuiLayer_ptr->begin();
			for (Layer* layer : pm_layerStack)
				layer->onImGuiRender();
			pm_imGuiLayer_ptr->end();

			// 🟢 프레임 종결: 커맨드버퍼 end + submit + present
			Renderer::endFrame();
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

		// ⚠️ 예전엔 true로 세운 직후 무조건 false로 덮어써서 최소화 판정이 죽어 있었다.
		//    그 상태로 리사이즈 경로가 붙으면 extent 0인 스왑체인을 만들려다 터진다.
		if (w == 0 || h == 0) {
			pm_minimized = true;
			return false;   // 최소화 중엔 재생성을 요청해봐야 못 만든다
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
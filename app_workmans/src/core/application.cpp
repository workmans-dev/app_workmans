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
		// 1. Create the window (builds the basic skeleton: Vulkan instance, device, etc.)
		pm_window_ptr = std::unique_ptr<Window>(Window::create());
		pm_window_ptr->setData_EventCallBack(BIND_EVENT_FN(App::onEvent));

		// 2. Initialize the renderer (this is where the core Vulkan parts -- RenderPass, DescriptorPool -- are finished)
		Renderer::init();

		// 3. Finally create the ImGui layer object (this is where the object lands in memory)
		pm_imGuiLayer_ptr = new ImGuiLayer(); // its onAttach() runs and creates a pure ImGui context
		pushOverlay(pm_imGuiLayer_ptr);

		// 4. Inject the Vulkan parts
		pm_imGuiLayer_ptr->InitVulkanBackend(
			static_cast<GLFWwindow*>(pm_window_ptr->getGLFWWindow()));
	}


	App::~App() {
		// WARNING: shutdown is all about order. Left to the default destructor order, RendererAPI,
		//    which has static lifetime, is destroyed later than the device and we get use-after-free.
		//    So it is all driven explicitly here.
		//
		//  1. Wait until the GPU finishes the last frame
		//     (required, because even the frame the close event arrived in gets submitted in full)
		RendererCommand::waitIdle();

		//  2. Destroy the layers -- MainWindow (RenderTarget/pipeline) first, then ImGuiLayer (Vulkan backend).
		//     The RenderTarget destructor calls ImGui_ImplVulkan_RemoveTexture, so it has to be this order.
		pm_layerStack.clear();

		//  3. RendererAPI resources -- the descriptor pool was ImGui's, so this comes after step 2
		Renderer::shutdown();

		//  4. VulkanContext (device/surface/instance) is destroyed by the pm_window_ptr destructor.
		//     Member destruction happens after this body ends, so it naturally comes last.
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

			// Event polling at the very top of the frame (minimizes input latency)
			pm_window_ptr->onUpdate();

			// Frame start: acquire + begin the command buffer.
			//    On failure (minimized / out-of-date) the whole frame is skipped.
			if (pm_minimized || !Renderer::beginFrame()) {
				continue;
			}

			// Offscreen render (each layer records the 3D scene into its own RenderTarget)
			for (Layer* layer : pm_layerStack)
				layer->onUpdate(timer);

			// ImGui (records the UI into the swapchain render pass)
			pm_imGuiLayer_ptr->begin();
			for (Layer* layer : pm_layerStack)
				layer->onImGuiRender();
			pm_imGuiLayer_ptr->end();

			// Frame end: command buffer end + submit + present
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

		// WARNING: this used to be set true and then unconditionally overwritten with false, which
		//    left the minimization check dead. With the resize path attached that blows up trying to build a swapchain with extent 0.
		if (w == 0 || h == 0) {
			pm_minimized = true;
			return false;   // while minimized, asking for recreation cannot build anything
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
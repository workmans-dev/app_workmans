#include "pch.h"
#include "application.h"
#include <glad/glad.h>
#include "renderer/renderer.h"
#include "input.h"

namespace AppWMs {
	App* App::pm_Instance = nullptr;

	App::App() {
		APP_WMS_CORE_ASSERTS(!pm_Instance, "Exits!!!")
		pm_Instance = this;
		std::cout << "created the App - Base class" << std::endl;
		// the Type is Window BUT it will return as the Win_win which is a parent Class of Window.
		// don't be confused here, so important!!
		pm_window_ptr = std::unique_ptr<Window>(Window::create());
		// set event call back
		pm_window_ptr->setData_EventCallBack(BIND_EVENT_FN(App::onEvent));
		pm_imGuiLayer_ptr = new ImGuiLayer();
		pushOverlay(pm_imGuiLayer_ptr);


		// test for triangle
		pm_vertexArray_ptr.reset(VertexArray::create());

		float vertexes[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};
		std::shared_ptr<VertexBuffer> pm_vertexBuffer_ptr;
		pm_vertexBuffer_ptr.reset( VertexBuffer::create(vertexes, sizeof(vertexes)));

		BufferLayout layout = {
			//BufferElements
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"}
		};
		pm_vertexBuffer_ptr->setLayout(layout);
		pm_vertexArray_ptr->addVertexBuffer(pm_vertexBuffer_ptr);

		unsigned int indexes[3] = { 0,1,2 };
		std::shared_ptr<IndexBuffer> pm_indexBuffer_ptr;
		pm_indexBuffer_ptr.reset(IndexBuffer::create(indexes, sizeof(indexes)/sizeof(uint32_t)));
		pm_vertexArray_ptr->setIndexBuffer(pm_indexBuffer_ptr);

		// shader
		std::string vertexSource = R"(
			#version 410 core
			layout(location=0) in vec3 a_Position;
			layout(location=1) in vec4 a_Color;
			out vec3 v_Position;
			out vec4 v_Color;
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);
			}
		)";
		std::string fragmentSource = R"(
			#version 410 core
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;
			void main()
			{
				color = v_Color;
			}
		)";
		pm_shader_ptr.reset(new Shader(vertexSource, fragmentSource));
	}

	App::~App() {
	}

	// send all events of layer events to Win_window : pm_window_ptr
	void App::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(App::onWindowClose));
		for (auto it = pm_layerStack.end(); it != pm_layerStack.begin();)
		{
			(*--it)->onEvent(event);
			if (event.p_handled)
				break;
		}
	}

	void App::run() {
		while (pm_running) 
		{
			// test for triangle
			RendererCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RendererCommand::clear();

			Renderer::beginScene();
			pm_shader_ptr->bind();
			pm_vertexArray_ptr->bind();
			Renderer::submit(pm_vertexArray_ptr);
			Renderer::end();
			glDrawElements(GL_TRIANGLES, pm_vertexArray_ptr->getIndexBuffer()->getCount(),
				GL_UNSIGNED_INT, nullptr);
			for (Layer* layer : pm_layerStack) 
			{
				layer->onUpdate();
			}

			pm_imGuiLayer_ptr->begin();
			for (Layer* layer : pm_layerStack)
			{
				layer->onImGuiRender();
			}
			pm_imGuiLayer_ptr->end();

			pm_window_ptr->onUpdate();


		}
	}

	bool App::onWindowClose(WindowCloseEvent& e) {
		pm_running = false;
		return true;
	}

	void App::pushLayer(Layer* layer) {
		pm_layerStack.pushLayer(layer);
		layer->onAttach();
	}

	void App::pushOverlay(Layer* overlay) {
		pm_layerStack.pushOverlay(overlay);
		overlay->onAttach();
	}

}
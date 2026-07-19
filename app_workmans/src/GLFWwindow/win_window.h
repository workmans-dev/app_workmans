#pragma once
#include "renderer/graphicContext.h"
#include "core/window.h"




namespace AppWMs {
	class Win_Window :public Window {
	private:
		GLFWwindow* m_GLFWwindow_ptr;
		GraphicContext* m_context_ptr;
		struct WindowData {
			std::string title;
			unsigned int w, h;
			bool VSync;
			EventCallBackFn EventCallBack;
		};
		WindowData m_data;
		
	private:
		virtual void init(const WindowProps& Win_props);
		virtual void close();

	public:
		Win_Window(const WindowProps& Win_props);
		virtual ~Win_Window();
		void onUpdate() override;
		inline unsigned int getWidth() const override {return m_data.w;}
		inline unsigned int getHeight() const override {return m_data.h;}
		inline void setData_EventCallBack(const EventCallBackFn& callback) override;
		void setVSync(bool enabled) override;
		bool isVSync() const override;
		inline virtual void* getGLFWWindow() const override { return m_GLFWwindow_ptr; }
	};
}
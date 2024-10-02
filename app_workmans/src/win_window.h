#pragma once
#include "window.h"
#include <GLFW/glfw3.h>

namespace AppWMs {
	class Win_Window :public Window {
	private:
		struct WindowData 
		{
			std::string title;
			unsigned int w, h;
			bool VSync;
			EventCallBackFn EventCallBack;
		};
		WindowData pm_data;
		GLFWwindow* pm_GLFWwindow_ptr;
	private:
		virtual void init(const WindowProps& Win_props);
		virtual void close();
	public:
		Win_Window(const WindowProps& Win_props);
		virtual ~Win_Window();
		void onUpdate() override;
		inline unsigned int getWidth() const override {return pm_data.w;}
		inline unsigned int getHeight() const override {return pm_data.h;}
		inline void setData_EventCallBack(const EventCallBackFn& callback) override { pm_data.EventCallBack = callback; }
		void setVSync(bool enabled) override;
		bool isVSync() const override;
		
	};
}
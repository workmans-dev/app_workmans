#pragma once
#include "pch.h"
#include "core/core.h"
#include "events/event.h"

namespace AppWMs {

	struct WindowProps 
	{
		std::string title;
		unsigned int w, h;
		WindowProps(const std::string & title_in="APP_WORKMANs", 
			unsigned int width_in = 1600, unsigned int height_in = 900):w(width_in), h(height_in), title(title_in){}
	};

	class  Window
	{
	public:
		using EventCallBackFn = std::function<void(Event&)>;
		virtual ~Window() {}
		virtual void onUpdate() = 0;
		virtual unsigned int getWidth() const = 0;
		virtual unsigned int getHeight() const = 0;

		virtual void setData_EventCallBack(const EventCallBackFn& callback) = 0;
		virtual void setVSync(bool enabled) = 0;
		virtual bool isVSync() const = 0;

		static Window* create(const WindowProps& Win_props = WindowProps());

		virtual void* getGLFWWindow() const = 0;
	};
}

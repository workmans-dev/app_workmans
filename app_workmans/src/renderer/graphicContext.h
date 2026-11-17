#pragma once
namespace AppWMs {
	class GraphicContext {
	public:
		// WARNING: required -- Win_Window holds this as a GraphicContext* and deletes it.
		//    Without virtual, the derived destructor (~VulkanContext) never runs and device/instance leak.
		virtual ~GraphicContext() = default;

		virtual void init() = 0;
		virtual bool beginFrame() = 0;   // acquire + begin the command buffer (false on failure -> skip the frame)
		virtual void swapBuffers() = 0;  // 🟢 end + submit + present
	};
}
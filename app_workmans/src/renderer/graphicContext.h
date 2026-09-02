#pragma once
namespace AppWMs {
	class GraphicContext {
	public:
		// ⚠️ 필수 — Win_Window가 GraphicContext* 로 들고 있다가 delete한다.
		//    가상이 아니면 파생 소멸자(~VulkanContext)가 호출되지 않아 device/instance가 샌다.
		virtual ~GraphicContext() = default;

		virtual void init() = 0;
		virtual bool beginFrame() = 0;   // 🟢 acquire + 커맨드버퍼 개시 (실패 시 false → 프레임 스킵)
		virtual void swapBuffers() = 0;  // 🟢 end + submit + present
	};
}
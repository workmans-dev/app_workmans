#pragma once
#include <stdint.h>

namespace AppWMs
{
	using MouseCode = uint16_t;

	// keyCode.h와 같은 방식 — 이 목록 하나에서 enum과 이름 사전이 함께 나온다.
	// keymap.yaml에는 여기 이름 앞에 Mouse를 붙여 쓴다: "MouseLeft", "MouseMiddle".
#define WMS_MOUSE_LIST(WMS_X)     \
	WMS_X(Button0,      0)        \
	WMS_X(Button1,      1)        \
	WMS_X(Button2,      2)        \
	WMS_X(Button3,      3)        \
	WMS_X(Button4,      4)        \
	WMS_X(Button5,      5)        \
	WMS_X(Button6,      6)        \
	WMS_X(Button7,      7)

	namespace Mouse
	{
		enum : MouseCode
		{
#define WMS_MOUSE_ENUM_ENTRY(name, value) name = value,
			WMS_MOUSE_LIST(WMS_MOUSE_ENUM_ENTRY)
#undef WMS_MOUSE_ENUM_ENTRY

			ButtonLast = Button7,
			// 별칭 — 사람이 읽는 이름. keymap.yaml에서도 이쪽을 쓴다.
			ButtonLeft = Button0,
			ButtonRight = Button1,
			ButtonMiddle = Button2
		};
	}
}

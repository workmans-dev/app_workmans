#pragma once
#include <stdint.h>

namespace AppWMs
{
	using MouseCode = uint16_t;

	// Same approach as keyCode.h -- the enum and the name dictionary both come from this one list.
	// In keymap.yaml these names are prefixed with Mouse: "MouseLeft", "MouseMiddle".
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
			// Aliases -- the human-readable names. keymap.yaml uses these too.
			ButtonLeft = Button0,
			ButtonRight = Button1,
			ButtonMiddle = Button2
		};
	}
}

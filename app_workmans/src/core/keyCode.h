#pragma once
#include <stdint.h>

namespace AppWMs
{
	using KeyCode = uint16_t;

	// ─────────────────────────────────────────────────────────────
	//  Key list -- the *single source of truth*
	//
	//  Both the enum below and keyMap.cpp's "string -> code" dictionary are generated from this list.
	//  Two copies of the list means adding a key by fixing only one side, which silently creates
	//  a key that exists in the enum but cannot be used in keymap.yaml. One list makes that impossible.
	//
	//  The names written here are exactly the names used in keymap.yaml ("LeftShift", "F1", "D0"...).
	//  The values came from glfw3.h.
	//
	//  WARNING: why the parameter is named WMS_X rather than the usual X -- there is an X key in the list.
	//     With the parameter named X, the X(X, 88) line gets its *second* X substituted too,
	//     producing nonsense like Key::<the macro name you passed>. That error was actually hit.
	//
	//  WARNING: events/keyCodes.h used to copy the GLFW macros verbatim (#define WMS_KEY_P 80),
	//     but a macro disappears in the preprocessor, so no name survives to runtime.
	//     That left no way to turn the "P" read from YAML back into a code. That file was removed.
	// ─────────────────────────────────────────────────────────────
#define WMS_KEY_LIST(WMS_X)          \
	WMS_X(Space,        32)          \
	WMS_X(Apostrophe,   39)          \
	WMS_X(Comma,        44)          \
	WMS_X(Minus,        45)          \
	WMS_X(Period,       46)          \
	WMS_X(Slash,        47)          \
	WMS_X(D0,           48)          \
	WMS_X(D1,           49)          \
	WMS_X(D2,           50)          \
	WMS_X(D3,           51)          \
	WMS_X(D4,           52)          \
	WMS_X(D5,           53)          \
	WMS_X(D6,           54)          \
	WMS_X(D7,           55)          \
	WMS_X(D8,           56)          \
	WMS_X(D9,           57)          \
	WMS_X(Semicolon,    59)          \
	WMS_X(Equal,        61)          \
	WMS_X(A,            65)          \
	WMS_X(B,            66)          \
	WMS_X(C,            67)          \
	WMS_X(D,            68)          \
	WMS_X(E,            69)          \
	WMS_X(F,            70)          \
	WMS_X(G,            71)          \
	WMS_X(H,            72)          \
	WMS_X(I,            73)          \
	WMS_X(J,            74)          \
	WMS_X(K,            75)          \
	WMS_X(L,            76)          \
	WMS_X(M,            77)          \
	WMS_X(N,            78)          \
	WMS_X(O,            79)          \
	WMS_X(P,            80)          \
	WMS_X(Q,            81)          \
	WMS_X(R,            82)          \
	WMS_X(S,            83)          \
	WMS_X(T,            84)          \
	WMS_X(U,            85)          \
	WMS_X(V,            86)          \
	WMS_X(W,            87)          \
	WMS_X(X,            88)          \
	WMS_X(Y,            89)          \
	WMS_X(Z,            90)          \
	WMS_X(LeftBracket,  91)          \
	WMS_X(Backslash,    92)          \
	WMS_X(RightBracket, 93)          \
	WMS_X(GraveAccent,  96)          \
	WMS_X(World1,       161)         \
	WMS_X(World2,       162)         \
	WMS_X(Escape,       256)         \
	WMS_X(Enter,        257)         \
	WMS_X(Tab,          258)         \
	WMS_X(Backspace,    259)         \
	WMS_X(Insert,       260)         \
	WMS_X(Delete,       261)         \
	WMS_X(Right,        262)         \
	WMS_X(Left,         263)         \
	WMS_X(Down,         264)         \
	WMS_X(Up,           265)         \
	WMS_X(PageUp,       266)         \
	WMS_X(PageDown,     267)         \
	WMS_X(Home,         268)         \
	WMS_X(End,          269)         \
	WMS_X(CapsLock,     280)         \
	WMS_X(ScrollLock,   281)         \
	WMS_X(NumLock,      282)         \
	WMS_X(PrintScreen,  283)         \
	WMS_X(Pause,        284)         \
	WMS_X(F1,           290)         \
	WMS_X(F2,           291)         \
	WMS_X(F3,           292)         \
	WMS_X(F4,           293)         \
	WMS_X(F5,           294)         \
	WMS_X(F6,           295)         \
	WMS_X(F7,           296)         \
	WMS_X(F8,           297)         \
	WMS_X(F9,           298)         \
	WMS_X(F10,          299)         \
	WMS_X(F11,          300)         \
	WMS_X(F12,          301)         \
	WMS_X(F13,          302)         \
	WMS_X(F14,          303)         \
	WMS_X(F15,          304)         \
	WMS_X(F16,          305)         \
	WMS_X(F17,          306)         \
	WMS_X(F18,          307)         \
	WMS_X(F19,          308)         \
	WMS_X(F20,          309)         \
	WMS_X(F21,          310)         \
	WMS_X(F22,          311)         \
	WMS_X(F23,          312)         \
	WMS_X(F24,          313)         \
	WMS_X(F25,          314)         \
	WMS_X(KP0,          320)         \
	WMS_X(KP1,          321)         \
	WMS_X(KP2,          322)         \
	WMS_X(KP3,          323)         \
	WMS_X(KP4,          324)         \
	WMS_X(KP5,          325)         \
	WMS_X(KP6,          326)         \
	WMS_X(KP7,          327)         \
	WMS_X(KP8,          328)         \
	WMS_X(KP9,          329)         \
	WMS_X(KPDecimal,    330)         \
	WMS_X(KPDivide,     331)         \
	WMS_X(KPMultiply,   332)         \
	WMS_X(KPSubtract,   333)         \
	WMS_X(KPAdd,        334)         \
	WMS_X(KPEnter,      335)         \
	WMS_X(KPEqual,      336)         \
	WMS_X(LeftShift,    340)         \
	WMS_X(LeftControl,  341)         \
	WMS_X(LeftAlt,      342)         \
	WMS_X(LeftSuper,    343)         \
	WMS_X(RightShift,   344)         \
	WMS_X(RightControl, 345)         \
	WMS_X(RightAlt,     346)         \
	WMS_X(RightSuper,   347)         \
	WMS_X(Menu,         348)

	namespace Key
	{
		enum : KeyCode
		{
#define WMS_KEY_ENUM_ENTRY(name, value) name = value,
			WMS_KEY_LIST(WMS_KEY_ENUM_ENTRY)
#undef WMS_KEY_ENUM_ENTRY
		};
	}
}

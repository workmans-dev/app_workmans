#pragma once
#include <string>
#include <unordered_map>
#include "core/keyCode.h"
#include "core/mouseCode.h"

namespace AppWMs {

	// ─────────────────────────────────────────────────────────────
	//  키맵 — 단축키를 코드에서 빼내 파일로 옮긴다.
	//
	//  왜 스크립트(파이썬/Lua)가 아니라 선언적 YAML인가:
	//    (1) yaml-cpp가 이미 서브모듈로 들어와 있다(.wmsa가 쓰는 것) → 새 의존성 0.
	//    (2) 스크립트면 **단축키를 알려면 코드를 실행해야 한다.** "현재 단축키 목록을
	//        UI에 뿌리기"나 "메뉴에 Ctrl+N 표시하기"가 사실상 불가능해진다.
	//        마야도 핫키는 선언적 파일이고, MEL/파이썬은 "눌렀을 때 실행될 명령"
	//        쪽에만 들어간다. 그 경계를 같게 잡았다.
	//    (3) YAML은 주석이 된다. 사람이 고쳐 쓰는 파일엔 "왜 이 키인지"를 적을 수
	//        있어야 한다 (JSON은 주석이 없다).
	//
	//  액션 이름은 "그룹.동작" 평탄화다: YAML의 중첩을 점으로 이어붙인다.
	//      gizmo:            →   "gizmo.translate"
	//        translate: W
	//
	//  ⚠️ 파일이 없거나 깨져도 앱은 뜬다 — 내장 기본값으로 떨어지고 경고만 찍는다.
	//     키맵 오타 하나로 편집기를 못 켜는 상황을 만들지 않는다.
	// ─────────────────────────────────────────────────────────────

	// 조합키. GLFW의 mods 비트와 같은 개념인데, 우리 쪽에서 직접 정의한다 —
	// 예전 WMS_MOD_* 매크로는 정의만 있고 아무도 안 써서 각자 손으로
	// IsKeyPressed(LeftControl) || IsKeyPressed(RightControl)을 반복하고 있었다.
	namespace KeyMod {
		enum : uint8_t {
			None = 0,
			Shift = 1 << 0,
			Ctrl = 1 << 1,
			Alt = 1 << 2,
			Super = 1 << 3
		};
	}

	struct KeyBinding {
		uint16_t code = 0;          // Key::* 또는 Mouse::*
		uint8_t  mods = KeyMod::None;
		bool     isMouse = false;
		bool     valid = false;     // 파일에 없던 액션이면 false

		// 사람이 읽는 표기 ("Ctrl+Shift+S"). 메뉴 라벨에 그대로 쓴다.
		std::string label;
	};

	class KeyMap {
	public:
		// 파일을 읽어 액션 표를 채운다. 실패하면 내장 기본값을 쓴다.
		// 경로는 실행 디렉토리 기준이다 (예: "assets/config/keymap.yaml").
		static void load(const std::string& filePath);

		// 액션의 바인딩. 없는 액션이면 valid == false인 것을 돌려준다.
		// 매 프레임 도는 경로(카메라 등)는 이걸 한 번 받아 캐시해서 쓸 것.
		static const KeyBinding& get(const std::string& action);

		// 키 이벤트가 이 액션인가. 조합키 상태까지 본다.
		// KeyPressedEvent 처리에서 쓴다.
		static bool isAction(const std::string& action, int keyCode);

		// 지금 눌려 있나(조합키 포함). 카메라처럼 누르고 있는 동안 계속 도는 입력용.
		static bool isHeld(const std::string& action);

		// 메뉴 라벨용. 없는 액션이면 빈 문자열.
		static const char* label(const std::string& action);

		// 현재 물려 있는 조합키 상태. 좌/우 키를 하나로 합쳐서 돌려준다.
		static uint8_t currentMods();

	private:
		static void loadDefaults();
		static std::unordered_map<std::string, KeyBinding> s_Bindings;
	};

}

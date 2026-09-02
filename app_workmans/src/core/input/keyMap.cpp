#include "pch.h"
#include "core/input/keyMap.h"
#include "core/input.h"
#include "yaml-cpp/yaml.h"

namespace AppWMs {

	std::unordered_map<std::string, KeyBinding> KeyMap::s_Bindings;

	// ─────────────────────────────────────────────────────────────
	//  이름 → 코드 사전
	//
	//  keyCode.h / mouseCode.h의 목록에서 그대로 뽑는다. 목록이 하나이므로
	//  "enum엔 있는데 YAML에선 못 쓰는 키"가 생길 수 없다.
	// ─────────────────────────────────────────────────────────────
	static const std::unordered_map<std::string, KeyCode>& keyNameTable()
	{
#define WMS_KEY_NAME_ENTRY(name, value) { #name, Key::name },
		static const std::unordered_map<std::string, KeyCode> table = {
			WMS_KEY_LIST(WMS_KEY_NAME_ENTRY)
		};
#undef WMS_KEY_NAME_ENTRY
		return table;
	}

	static const std::unordered_map<std::string, MouseCode>& mouseNameTable()
	{
#define WMS_MOUSE_NAME_ENTRY(name, value) { "Mouse" #name, Mouse::name },
		static const std::unordered_map<std::string, MouseCode> table = {
			WMS_MOUSE_LIST(WMS_MOUSE_NAME_ENTRY)
#undef WMS_MOUSE_NAME_ENTRY
			// 사람이 쓰는 별칭. Button0/1/2보다 이쪽이 먼저 떠오른다.
			{ "MouseLeft",   Mouse::ButtonLeft   },
			{ "MouseRight",  Mouse::ButtonRight  },
			{ "MouseMiddle", Mouse::ButtonMiddle },
		};
		return table;
	}

	// "Ctrl+Shift+S" → mods = Ctrl|Shift, key = S
	// 마지막 토큰이 키고, 앞의 것들은 전부 조합키다.
	static bool parseBinding(const std::string& text, KeyBinding& out)
	{
		out = KeyBinding{};
		if (text.empty())
			return false;

		std::vector<std::string> tokens;
		size_t start = 0;
		while (true) {
			const size_t plus = text.find('+', start);
			std::string token = text.substr(start, plus == std::string::npos ? std::string::npos : plus - start);
			// 앞뒤 공백 제거 — "Ctrl + N"처럼 써도 받아준다
			const size_t b = token.find_first_not_of(" \t");
			const size_t e = token.find_last_not_of(" \t");
			if (b != std::string::npos)
				tokens.push_back(token.substr(b, e - b + 1));
			if (plus == std::string::npos)
				break;
			start = plus + 1;
		}
		if (tokens.empty())
			return false;

		for (size_t i = 0; i + 1 < tokens.size(); ++i) {
			const std::string& m = tokens[i];
			if (m == "Ctrl" || m == "Control")   out.mods |= KeyMod::Ctrl;
			else if (m == "Shift")               out.mods |= KeyMod::Shift;
			else if (m == "Alt")                 out.mods |= KeyMod::Alt;
			else if (m == "Super" || m == "Win") out.mods |= KeyMod::Super;
			else {
				APP_WMS_CORE_WARN("keymap: 모르는 조합키 '{0}' (바인딩 '{1}')", m, text);
				return false;
			}
		}

		const std::string& keyName = tokens.back();
		const auto& mouseTable = mouseNameTable();
		if (auto it = mouseTable.find(keyName); it != mouseTable.end()) {
			out.code = it->second;
			out.isMouse = true;
		}
		else {
			const auto& keyTable = keyNameTable();
			auto kt = keyTable.find(keyName);
			if (kt == keyTable.end()) {
				APP_WMS_CORE_WARN("keymap: 모르는 키 이름 '{0}' (바인딩 '{1}')", keyName, text);
				return false;
			}
			out.code = kt->second;
			out.isMouse = false;
		}

		out.label = text;
		out.valid = true;
		return true;
	}

	// YAML의 중첩을 "그룹.동작"으로 평탄화한다.
	// 스칼라를 만나면 바인딩, 맵을 만나면 접두사를 이어붙여 한 단계 더 내려간다.
	static void flatten(const YAML::Node& node, const std::string& prefix,
		std::unordered_map<std::string, KeyBinding>& out)
	{
		if (!node.IsMap())
			return;

		for (const auto& entry : node) {
			const std::string key = entry.first.as<std::string>();
			const std::string path = prefix.empty() ? key : prefix + "." + key;
			const YAML::Node& value = entry.second;

			if (value.IsMap()) {
				flatten(value, path, out);
			}
			else if (value.IsScalar()) {
				KeyBinding binding;
				if (parseBinding(value.as<std::string>(), binding))
					out[path] = binding;
			}
		}
	}

	void KeyMap::loadDefaults()
	{
		// 파일을 못 읽었을 때의 최후 방어선. 여기 값은 assets/config/keymap.yaml의
		// 기본 내용과 같아야 한다 — 다르면 "파일 지웠더니 키가 바뀌는" 혼란이 생긴다.
		static const std::pair<const char*, const char*> defaults[] = {
			{ "gizmo.none",        "Q" },
			{ "gizmo.translate",   "W" },
			{ "gizmo.rotate",      "E" },
			{ "gizmo.scale",       "R" },
			{ "file.newScene",     "Ctrl+N" },
			{ "file.openScene",    "Ctrl+O" },
			{ "file.saveAsScene",  "Ctrl+Shift+S" },
			{ "app.exit",          "Ctrl+Q" },
			{ "hierarchy.parent",  "P" },
			{ "hierarchy.unparent","Shift+P" },
			{ "edit.delete",       "Delete" },
			{ "visibility.hide",   "Ctrl+H" },
			{ "visibility.show",   "Shift+H" },
			{ "camera.modifier",   "LeftAlt" },
			{ "camera.tumble",     "MouseLeft" },
			{ "camera.track",      "MouseMiddle" },
			{ "camera.dolly",      "MouseRight" },
		};

		for (const auto& [action, text] : defaults) {
			KeyBinding binding;
			if (parseBinding(text, binding))
				s_Bindings[action] = binding;
		}
	}

	void KeyMap::load(const std::string& filePath)
	{
		s_Bindings.clear();

		std::ifstream file(filePath);
		if (!file.is_open()) {
			APP_WMS_CORE_WARN("keymap: '{0}'을 열 수 없다 — 내장 기본값을 쓴다.", filePath);
			loadDefaults();
			return;
		}

		try {
			std::stringstream buffer;
			buffer << file.rdbuf();
			const YAML::Node root = YAML::Load(buffer.str());
			flatten(root, "", s_Bindings);
		}
		catch (const std::exception& e) {
			// 오타 하나로 편집기를 못 켜면 안 된다. 경고만 남기고 기본값으로 간다.
			APP_WMS_CORE_ERROR("keymap: '{0}' 파싱 실패 ({1}) — 내장 기본값을 쓴다.",
				filePath, e.what());
			s_Bindings.clear();
			loadDefaults();
			return;
		}

		if (s_Bindings.empty()) {
			APP_WMS_CORE_WARN("keymap: '{0}'에서 읽은 바인딩이 없다 — 내장 기본값을 쓴다.", filePath);
			loadDefaults();
			return;
		}
		APP_WMS_CORE_INFO("keymap: '{0}' 로드 — 바인딩 {1}개", filePath, s_Bindings.size());
	}

	const KeyBinding& KeyMap::get(const std::string& action)
	{
		static const KeyBinding invalid{};
		auto it = s_Bindings.find(action);
		return it == s_Bindings.end() ? invalid : it->second;
	}

	uint8_t KeyMap::currentMods()
	{
		// 좌/우를 하나로 합친다. 예전엔 호출부마다
		// IsKeyPressed(LeftControl) || IsKeyPressed(RightControl)을 손으로 썼다.
		uint8_t mods = KeyMod::None;
		if (Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift))
			mods |= KeyMod::Shift;
		if (Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl))
			mods |= KeyMod::Ctrl;
		if (Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt))
			mods |= KeyMod::Alt;
		if (Input::IsKeyPressed(Key::LeftSuper) || Input::IsKeyPressed(Key::RightSuper))
			mods |= KeyMod::Super;
		return mods;
	}

	bool KeyMap::isAction(const std::string& action, int keyCode)
	{
		const KeyBinding& binding = get(action);
		if (!binding.valid || binding.isMouse)
			return false;
		if (binding.code != (uint16_t)keyCode)
			return false;
		// ⚠️ 정확히 일치를 요구한다. 부분 일치를 허용하면 P가 Shift+P를 잡아먹어
		//    unparent가 절대 안 걸린다 — 둘이 같은 키를 쓰는 게 정확히 이 경우다.
		return currentMods() == binding.mods;
	}

	bool KeyMap::isHeld(const std::string& action)
	{
		const KeyBinding& binding = get(action);
		if (!binding.valid)
			return false;

		const bool down = binding.isMouse
			? Input::IsMousePressed(binding.code)
			: Input::IsKeyPressed(binding.code);
		if (!down)
			return false;

		// 조합키를 요구하는 바인딩만 조합키를 검사한다. 조합키가 없는 바인딩은
		// 다른 키가 같이 눌려 있어도 성립한다(카메라 조작 중 다른 키를 눌러도 안 끊긴다).
		if (binding.mods == KeyMod::None)
			return true;
		return (currentMods() & binding.mods) == binding.mods;
	}

	const char* KeyMap::label(const std::string& action)
	{
		const KeyBinding& binding = get(action);
		return binding.valid ? binding.label.c_str() : "";
	}

}

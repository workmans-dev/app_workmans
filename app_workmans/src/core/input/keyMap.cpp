#include "pch.h"
#include "core/input/keyMap.h"
#include "core/input.h"
#include "yaml-cpp/yaml.h"

namespace AppWMs {

	std::unordered_map<std::string, KeyBinding> KeyMap::s_Bindings;

	// ─────────────────────────────────────────────────────────────
	//  name -> code dictionary
	//
	//  Taken straight from the lists in keyCode.h / mouseCode.h. With one list there can be no
	//  "key that exists in the enum but cannot be used in YAML".
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
			// The aliases people actually use. These come to mind before Button0/1/2.
			{ "MouseLeft",   Mouse::ButtonLeft   },
			{ "MouseRight",  Mouse::ButtonRight  },
			{ "MouseMiddle", Mouse::ButtonMiddle },
		};
		return table;
	}

	// "Ctrl+Shift+S" → mods = Ctrl|Shift, key = S
	// The last token is the key; everything before it is a modifier.
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
			// Trim surrounding whitespace -- "Ctrl + N" is accepted too
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
				APP_WMS_CORE_WARN("keymap: unknown modifier '{0}' (binding '{1}')", m, text);
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
				APP_WMS_CORE_WARN("keymap: unknown key name '{0}' (binding '{1}')", keyName, text);
				return false;
			}
			out.code = kt->second;
			out.isMouse = false;
		}

		out.label = text;
		out.valid = true;
		return true;
	}

	// Flattens the YAML nesting into "group.action".
	// A scalar is a binding; a map appends to the prefix and descends one more level.
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
		// The last line of defense when the file cannot be read. The values here have to match the
		// defaults in assets/config/keymap.yaml -- otherwise you get "I deleted the file and the keys changed".
		static const std::pair<const char*, const char*> defaults[] = {
			{ "gizmo.none",        "Q" },
			{ "gizmo.translate",   "W" },
			{ "gizmo.rotate",      "E" },
			{ "gizmo.scale",       "R" },
			{ "file.newScene",     "Ctrl+N" },
			{ "file.openScene",    "Ctrl+O" },
			{ "file.saveAsScene",  "Ctrl+Shift+S" },
			{ "app.exit",          "Ctrl+Q" },
			// Selection -- the Maya convention. A click with no modifier (= replace) needs no binding.
			{ "select.toggle",     "Shift+MouseLeft" },
			{ "select.subtract",   "Ctrl+MouseLeft" },
			{ "select.add",        "Ctrl+Shift+MouseLeft" },
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
			APP_WMS_CORE_WARN("keymap: cannot open '{0}' - using the built-in defaults.", filePath);
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
			// One typo must not stop the editor opening. Log a warning and fall back to the defaults.
			APP_WMS_CORE_ERROR("keymap: failed to parse '{0}' ({1}) - using the built-in defaults.",
				filePath, e.what());
			s_Bindings.clear();
			loadDefaults();
			return;
		}

		if (s_Bindings.empty()) {
			APP_WMS_CORE_WARN("keymap: no bindings read from '{0}' - using the built-in defaults.", filePath);
			loadDefaults();
			return;
		}
		APP_WMS_CORE_INFO("keymap: loaded '{0}' - {1} bindings", filePath, s_Bindings.size());
	}

	const KeyBinding& KeyMap::get(const std::string& action)
	{
		static const KeyBinding invalid{};
		auto it = s_Bindings.find(action);
		return it == s_Bindings.end() ? invalid : it->second;
	}

	uint8_t KeyMap::currentMods()
	{
		// Merges left and right into one. Call sites used to hand-write
		// IsKeyPressed(LeftControl) || IsKeyPressed(RightControl).
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
		// WARNING: an exact match is required. Allowing a partial match would let P swallow Shift+P
		//    so unparent never fires -- and those two sharing a key is exactly this case.
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

		// Only a binding that requires modifiers checks modifiers. A binding with none holds even
		// if other keys are down too (pressing another key during camera manipulation does not break it).
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

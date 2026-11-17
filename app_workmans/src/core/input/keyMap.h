#pragma once
#include <string>
#include <unordered_map>
#include "core/keyCode.h"
#include "core/mouseCode.h"

namespace AppWMs {

	// ─────────────────────────────────────────────────────────────
	//  Keymap -- pulls shortcuts out of the code and into a file.
	//
	//  Why declarative YAML rather than a script (Python/Lua):
	//    (1) yaml-cpp is already a submodule (the one .wmsa uses) -> zero new dependencies.
	//    (2) With a script, *knowing a shortcut means running code.* "List the current
	//        shortcuts in the UI" or "show Ctrl+N in the menu" becomes effectively impossible.
	//        Maya's hotkeys are a declarative file too, and MEL/Python only goes on the
	//        "command to run when pressed" side. The same boundary is drawn here.
	//    (3) YAML takes comments. A file humans edit by hand has to be able to say "why this
	//        key" (JSON has no comments).
	//
	//  Action names are a flattened "group.action": the YAML nesting is joined with dots.
	//      gizmo:            →   "gizmo.translate"
	//        translate: W
	//
	//  WARNING: the app starts even if the file is missing or broken -- it falls back to the
	//     built-in defaults and only logs a warning. One keymap typo must not stop the editor opening.
	// ─────────────────────────────────────────────────────────────

	// Modifier keys. The same concept as GLFW's mods bits, but defined on our side --
	// the old WMS_MOD_* macros were defined and used by nobody, so everyone was hand-repeating
	// IsKeyPressed(LeftControl) || IsKeyPressed(RightControl).
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
		uint16_t code = 0;          // Key::* or Mouse::*
		uint8_t  mods = KeyMod::None;
		bool     isMouse = false;
		bool     valid = false;     // false when the action was not in the file

		// Human-readable form ("Ctrl+Shift+S"). Used directly as the menu label.
		std::string label;
	};

	class KeyMap {
	public:
		// Reads the file and fills the action table. On failure the built-in defaults are used.
		// The path is relative to the working directory (e.g. "assets/config/keymap.yaml").
		static void load(const std::string& filePath);

		// An action's binding. For a missing action it returns one with valid == false.
		// A path that runs every frame (the camera, say) should take this once and cache it.
		static const KeyBinding& get(const std::string& action);

		// Is this key event that action? It checks the modifier state too.
		// Used when handling KeyPressedEvent.
		static bool isAction(const std::string& action, int keyCode);

		// Is it held down right now (modifiers included)? For input that runs while held, like the camera.
		static bool isHeld(const std::string& action);

		// For menu labels. An empty string for a missing action.
		static const char* label(const std::string& action);

		// The modifier state currently held. Left/right keys are merged into one.
		static uint8_t currentMods();

	private:
		static void loadDefaults();
		static std::unordered_map<std::string, KeyBinding> s_Bindings;
	};

}

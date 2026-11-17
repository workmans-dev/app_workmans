#pragma once

#include "core/core.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include <vector>


namespace AppWMs {

	// How selection works -- the Maya convention. The outliner and the viewport use the *same table*.
	// If the rules differ per panel, hands cannot learn them.
	//
	//   click              Replace   drop the existing selection and take only this
	//   Shift+click        Toggle    remove it if selected, otherwise add it
	//   Ctrl+click         Subtract  remove only
	//   Ctrl+Shift+click   Add       add only
	//
	// WARNING: the actual modifiers are not here -- the select group in keymap.yaml decides them.
	enum class SelectMode { Replace, Toggle, Add, Subtract };

	class SceneProperties {
	public:
		SceneProperties() = default;
		SceneProperties(const Ref<Scene>& scene);
		void onImGuiRender();
		void setScene(const Ref<Scene>& scene);
		// The Property editor itself -- name + tab bar.
		// The tabs are not screen tidying but a *boundary*. Object (the thing itself) and
		// Modifier (the operations applied to it) handle different kinds of entity and must not mix.
		void showPropertyComponents( Entity& entity);
		// Draws one node, and recurses into the children when it is expanded.
		// WARNING: taken by value -- the recursion makes temporary child Entity objects, so a reference cannot bind.
		void drawEntityNode(Entity entity);
		void set_vis_color(Entity& entity);

		// The key event path. Returns true when handled.
		//
		// WARNING: this used to poll ImGui::IsKeyPressed directly every frame. That (1) bakes the
		//    key into the code rather than keymap.yaml, and (2) uses the ImGuiKey enum, which
		//    diverges from our key code scheme again. Now it takes the same event path as other shortcuts.
		bool onKeyPressed(int keyCode);

		// WARNING: a vector, not a set -- because *the selection order is information*.
		//    The Maya convention: pick the children first, pick the parent last, then P.
		//    std::set would sort by entity id and throw that order away wholesale.
		//    (which is why EntityComparator is no longer used here)
		std::vector<Entity>& get_selectedEntities() { return pm_selectedEntities; }

		// The *active* object among the selected ones = the one picked last.
		// In Maya this is what the manipulator and the attribute editor attach to, and
		// it is also what parent makes the parent. Keeping it to one avoids confusion.
		Entity* get_activeEntity();

		// Which selection mode the currently held modifiers mean. It queries the select group in
		// keymap.yaml, so changing a key changes the outliner and the viewport *together*.
		static SelectMode selectModeFromMods();

		// Applies the viewport picking (P-b) result to the selection.
		//    A handle of entt::null means empty space was clicked.
		//
		// WARNING: mode has to be the one from *the moment of the click*. The readback arrives a few
		//    frames later, so reading the modifiers again then may find them already released.
		void applyPickSelection(entt::entity handle, SelectMode mode);

	private:
		// Selection handling -- reused as is even when the click handler moves inside the tree traversal (K-b).
		void drawNameField(Entity& entity);
		void drawObjectTab(Entity& entity);     // the thing itself -- Transform / Visibility / Camera / Material
		void drawModifierTab(Entity& entity);   // operations applied to it -- Mirror / Skin ... (not implemented)

		bool isSelected(const Entity& e) const;
		void handleSelectionClick(const Entity& e, SelectMode mode);
		// Sweeps handles that vanished through deletion out of the selection list.
		void pruneInvalidSelection();
		void deleteSelected();
		void setSelectedVisible(bool visible);

		// Hierarchy operations -- the Maya convention (children first, parent last, then P)
		void parentSelected();
		void unparentSelected();

		// WARNING: a drop must not be handled on the spot. drawEntityNode is mid-recursion, and if
		//    setParent tears up the parent/child vectors, the position being iterated dies right there.
		//    Only the request is recorded, and it is applied after the whole tree is drawn.
		void applyPendingReparent();

	private:
		Ref<Scene> pm_scene;
		std::vector<Entity> pm_selectedEntities;

		// A deferred drop request. target == entt::null means unparent to the root.
		bool         pm_hasPendingReparent = false;
		entt::entity pm_dragSource{ entt::null };
		entt::entity pm_dropTarget{ entt::null };

	private:
		bool editMode = false;
		char editBuffer[256] = "";
		bool clickedOnEntity = false;
	};

}
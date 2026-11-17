#include "pch.h"
#include <imgui/imgui.h>
#include <imGui/imgui_internal.h>
#include "sceneProperty.h"
#include "scene/components/components.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ranges>


namespace AppWMs {
	template<typename T, typename ui_buildFunc>
	static void DrawComponent(const std::string& label, Entity& entity, ui_buildFunc uiFunc, const int& componentCount) {
		if (entity.hasComponent<T>()) {
			auto& comp = entity.getComponent<T>();
			if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
				// Child frame for the DragFloat3
				std::string group_label = label + "Group";
				// Calculate dynamic height based on number of components
				float lineHeight = ImGui::GetFrameHeightWithSpacing(); // Get single UI element height
				float totalHeight = componentCount * lineHeight; // 10.0f for padding
				const float& ui_x = ImGui::GetContentRegionAvail().x;
				ImGui::BeginChild(group_label.c_str(), ImVec2(ui_x, lineHeight), ImGuiChildFlags_ResizeY,
					ImGuiWindowFlags_NoScrollbar);
				uiFunc(comp);
				ImGui::EndChild();
				ImGui::TreePop();  // Pop the collapsible group
			}
		}
	}

	static void DrawVec3(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 66.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::Text(label.c_str());

		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() + 10.f);
		float lineHeight = GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		int i = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		std::array<const char*, 3> translate_comp{ "X", "Y", "Z" };
		std::array<float*, 3> values_comp{ &values.x, &values.y, &values.z };
		std::array<ImVec4, 3> RGB{ ImVec4(0.78f, 0.25f, 0.15f, 1.0f),ImVec4(0.3f, 0.78f, 0.2f, 1.0f),ImVec4(0.3f, 0.4f, 0.9f, 1.0f) };
		auto keyIt = translate_comp.begin();
		auto valueIt = values_comp.begin();
		auto colorIt = RGB.begin();

		while (keyIt != translate_comp.end() && valueIt != values_comp.end()) {
			ImGui::PushFont(boldFont);
			ImGui::PushStyleColor(ImGuiCol_Text, *colorIt);
			if (ImGui::Button(*keyIt, buttonSize))
				**valueIt = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor();

			char hash_label[32];
			snprintf(hash_label, sizeof(hash_label), "##%s", *keyIt);
			ImGui::SameLine();
			ImGui::DragFloat(hash_label, *valueIt, 0.01f);
			ImGui::PopItemWidth();
			if (i < 2)
				ImGui::SameLine();
			++keyIt;
			++valueIt;
			++colorIt;
			++i;
		}
		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}
	// Quaternion editor. Unlike DrawVec3 it does not use axis colors --
	// w/x/y/z are 4D components, not axes, so attaching the RGB=XYZ convention would be a lie.
	//
	// WARNING: dragging a component directly here takes the length away from 1. Normalization
	//    happens when getLocalMatrix() reads it (safeNormalizeQuat), so the image does not break,
	//    but the displayed value can diverge from what is applied. The normal path is the gizmo.
	static void DrawQuat(const std::string& label, glm::quat& q, float columnWidth = 66.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::Text(label.c_str());
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth() + 10.f);
		float lineHeight = GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		std::array<const char*, 4> names{ "W", "X", "Y", "Z" };
		std::array<float*, 4> values{ &q.w, &q.x, &q.y, &q.z };
		for (int i = 0; i < 4; ++i) {
			ImGui::PushFont(boldFont);
			// The button resets to the identity quaternion (W is 1, the rest 0)
			if (ImGui::Button(names[i], buttonSize))
				q = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			ImGui::PopFont();

			char hash_label[32];
			snprintf(hash_label, sizeof(hash_label), "##q%s", names[i]);
			ImGui::SameLine();
			ImGui::DragFloat(hash_label, values[i], 0.01f);
			ImGui::PopItemWidth();
			if (i < 3)
				ImGui::SameLine();
		}
		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	static void DrawBool(int& outValue) {
		ImGui::Text("Visibility:");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(100);

		if (ImGui::DragInt("##Visibility", &outValue, 1, 0, 1, outValue == 1 ? "On" : "Off")) {
			outValue = outValue > 0 ? 1 : 0; // Clamping to 0 or 1
		}
	}

	SceneProperties::SceneProperties(const Ref<Scene>& scene) :pm_scene(scene)
	{

	}
	bool SceneProperties::isSelected(const Entity& e) const
	{
		return std::find(pm_selectedEntities.begin(), pm_selectedEntities.end(), e)
			!= pm_selectedEntities.end();
	}

	Entity* SceneProperties::get_activeEntity()
	{
		// The one picked last is the active one (the Maya convention).
		return pm_selectedEntities.empty() ? nullptr : &pm_selectedEntities.back();
	}

	// Modifiers -> selection mode. The select group in keymap.yaml decides.
	//
	// WARNING: judged by an *exact match*. With a partial match, Ctrl (subtract) would swallow
	//    Ctrl+Shift (add) first and add would never fire -- isAction is exact for the same reason.
	SelectMode SceneProperties::selectModeFromMods()
	{
		const uint8_t mods = KeyMap::currentMods();
		if (mods == KeyMod::None)
			return SelectMode::Replace;   // a click with no modifier needs no binding

		// The order does not matter -- with an exact match the three values do not overlap.
		const KeyBinding& add = KeyMap::get("select.add");
		if (add.valid && mods == add.mods)       return SelectMode::Add;
		const KeyBinding& sub = KeyMap::get("select.subtract");
		if (sub.valid && mods == sub.mods)       return SelectMode::Subtract;
		const KeyBinding& tog = KeyMap::get("select.toggle");
		if (tog.valid && mods == tog.mods)       return SelectMode::Toggle;

		// An unknown combination (Alt = the camera modifier, say) does not fall through to replace.
		// With wiping the selection as the default, the cost of a misclick would be too high.
		return SelectMode::Toggle;
	}

	void SceneProperties::handleSelectionClick(const Entity& e, SelectMode mode)
	{
		auto it = std::find(pm_selectedEntities.begin(), pm_selectedEntities.end(), e);
		const bool already = (it != pm_selectedEntities.end());

		switch (mode) {
		case SelectMode::Replace:
			pm_selectedEntities.clear();
			pm_selectedEntities.push_back(e);
			return;

		case SelectMode::Add:
			// If it is already in, leave it where it is -- moving it to the back would silently change
			// the active target and shake parent's "the one picked last is the parent" rule.
			if (!already) pm_selectedEntities.push_back(e);
			return;

		case SelectMode::Subtract:
			if (already) pm_selectedEntities.erase(it);
			return;

		case SelectMode::Toggle:
			break;   // falls into the common path below
		}

		if (!already) {
			// WARNING: always append at the *back*. This position is the selection order, and
			//    the last element becomes the active one (= parent's parent).
			pm_selectedEntities.push_back(e);
		}
		else {
			// Even when erase shifts things forward, the relative order of the rest is preserved.
			pm_selectedEntities.erase(it);
		}
	}

	// Applies the viewport picking (P-b) result to the selection. The key is to run it through
	// the *same function* as an outliner click -- two paths with their own logic would diverge.
	void SceneProperties::applyPickSelection(entt::entity handle, SelectMode mode)
	{
		// -- click on empty space
		if (handle == entt::null) {
			// Pressing empty space while holding a modifier does nothing.
			// A near miss during a multi-selection must not wipe the whole selection (Maya's behavior).
			if (mode == SelectMode::Replace)
				pm_selectedEntities.clear();
			return;
		}

		// WARNING: a few frames pass between the click and the readback. The entity may have been
		//    deleted in between, so a dead handle never goes into the selection (it would blow up the Property editor).
		Entity picked{ handle, pm_scene.get() };
		if (!pm_scene->isValid(picked))
			return;

		handleSelectionClick(picked, mode);
	}

	void SceneProperties::pruneInvalidSelection()
	{
		// Because of subtree deletion (the Maya way), an entity I never deleted can disappear --
		// deleting a parent kills its descendants. Calling getComponent on a dead handle blows up.
		pm_selectedEntities.erase(
			std::remove_if(pm_selectedEntities.begin(), pm_selectedEntities.end(),
				[this](const Entity& e) { return !pm_scene->isValid(e); }),
			pm_selectedEntities.end());
	}

	void SceneProperties::onImGuiRender() {
		ImGui::Begin("Scene properties");

		// Swept once before drawing. Handles killed by last frame's deletion may still be in there.
		pruneInvalidSelection();

		// Check if the left mouse button is clicked in the window
		bool clickedInsidePanel = ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

		clickedOnEntity = false;
		// WARNING: view<TransformComponent> -- it walks *objects only*.
		//    The outliner is a list of "things placed in the scene". A position-less entity such as
		//    a modifier must not show up here (Blender does not list modifiers as outliner items
		//    either, and Maya defaults to "Show DAG objects only"). Those are handled in their own
		//    tab, and this traversal filters them out by component composition alone.
		//    As a bonus it makes set_vis_color below safe to assume a VisComponent unconditionally.
		//
		// Only the roots are walked. Each node recurses into its own children --
		// walking everything here would show a child once under its parent and once at the top level.
		for (auto e_id : pm_scene->pm_register.view<TransformComponent>()) {
			Entity entity{ e_id, pm_scene.get() };
			if (pm_scene->getParent(entity))
				continue;
			drawEntityNode(entity);
		}
		// The empty space left under the tree is a drop target too -- dropping here unparents to the root.
		// The same behavior as Maya's outliner, and it gives a mouse path out of the hierarchy besides the shortcut.
		const ImVec2 remaining = ImGui::GetContentRegionAvail();
		if (remaining.x > 0.0f && remaining.y > 0.0f) {
			ImGui::Dummy(remaining);
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("WMS_ENTITY")) {
					pm_dragSource = *(const entt::entity*)p->Data;
					pm_dropTarget = entt::null;      // no parent = to the root
					pm_hasPendingReparent = true;
				}
				ImGui::EndDragDropTarget();
			}
		}

		if (clickedInsidePanel && !clickedOnEntity) {
			// Only deselect if there were selected entities
			if (!pm_selectedEntities.empty()) {
				pm_selectedEntities.clear();
				std::cout << "Deselected all entities!" << std::endl;
			}
		}
		ImGui::End();

		// The hierarchy is changed only after the tree traversal is fully done, for the reason in the comment above.
		applyPendingReparent();

		ImGui::Begin("Property editor");
		// Shows the properties of the one picked last (the same rule as Maya's attribute editor).
		if (Entity* active = get_activeEntity())
			showPropertyComponents(*active);
		ImGui::End();

	}

	void SceneProperties::setScene(const Ref<Scene>& scene)
	{
		pm_scene = scene;
		pm_selectedEntities.clear();
	}


	// The tab bar host.
	//
	// WARNING: the tabs are split not to tidy the screen but *to pin the boundary down in code*.
	//    Object (the thing itself) and Modifier (the operations applied to it) handle different
	//    kinds of entity -- an object has a position and a modifier does not.
	//    Mixed into one panel, a flow appears where "add component" creates a modifier, and at
	//    that moment the distinction collapses. The wall goes up now.
	//
	// The Data tab (the shape itself) arrives when Ref<Mesh> exists -- built now it would hold nothing.
	void SceneProperties::showPropertyComponents( Entity& entity)
	{
		// The name sits outside the tabs. Whatever tab is open, "what am I looking at" must always show.
		drawNameField(entity);

		if (ImGui::BeginTabBar("##PropertyTabs")) {
			if (ImGui::BeginTabItem("Object")) {
				drawObjectTab(entity);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Modifier")) {
				drawModifierTab(entity);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	void SceneProperties::drawNameField(Entity& entity)
	{
		if (!entity.hasComponent<TagComponent>())
			return;

		char buffer[256];
		TagComponent& tagComp = entity.getComponent<TagComponent>();
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), tagComp.name.c_str());
		if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			tagComp.name = std::string(buffer); // Update item name when the input is changed
		}
		ImGui::Separator();
	}

	void SceneProperties::drawModifierTab(Entity& entity)
	{
		// There is no actual modifier yet. This tab is the boundary, put up first.
		//    Mirror, skin and so on are created as *position-less entities* (Scene::createEntity)
		//    and show up in this list. They are not stacked as components on an object entity --
		//    that is what lets them be evaluated by contiguous per-kind traversal (view<MirrorComponent> etc.).
		//    Going Blender's way, with a heap-linked list per object, would lose that benefit.
		// WARNING: on-screen strings are written in ASCII. The font is Rubik (Latin only) and the
		//    glyph range is the default, so Korean comes out blank. A Korean UI would need a Korean
		//    font plus GetGlyphRangesKorean() added to imGuiLayer's font load. (unresolved)
		ImGui::TextDisabled("No modifiers.");
		ImGui::Spacing();
		ImGui::TextWrapped("Mirror / Skin / Subdiv will stack here. (not implemented)");
	}

	void SceneProperties::drawObjectTab(Entity& entity)
	{
		DrawComponent<TransformComponent>("Transform", entity, [&](auto& transComp) {
			DrawVec3("Translate", transComp.translate);

			// Rotation mode -- the Blender way. Picking one changes only the representation; the rotation is kept
			// (the conversion happens inside setRotationMode).
			const RotationMode mode = transComp.getRotationMode();
			ImGui::Columns(2);
			ImGui::Text("Rot Mode");
			ImGui::SetColumnWidth(0, 66.0f);
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##RotMode", rotationModeName(mode))) {
				for (int i = 0; i <= (int)RotationMode::Quaternion; ++i) {
					const RotationMode m = (RotationMode)i;
					if (i == (int)RotationMode::Quaternion)
						ImGui::Separator();   // the six Euler orders and the quaternion differ in character
					if (ImGui::Selectable(rotationModeName(m), m == mode))
						transComp.setRotationMode(m);
				}
				ImGui::EndCombo();
			}
			ImGui::Columns(1);

			if (mode == RotationMode::Quaternion)
				DrawQuat("Rotate", transComp.rotateQuat);
			else
				DrawVec3("Rotate", transComp.rotate);

			DrawVec3("Scale", transComp.scale, 1.0f);
			}, 4);
		DrawComponent<VisComponent>("Visibility", entity, [&](auto& visComp) {
			DrawBool(visComp.visible);
			}, 1);

		DrawComponent<CameraMatrixComponent>("Camera", entity, [&](auto& camera) {
			const char* cameraType[] = { "perspective", "orthographic" };
			const char* currentCamereTypeString = cameraType[static_cast<int>(camera.sceneCam.get_camProjType())];
			if (ImGui::BeginCombo("##Camera Type", currentCamereTypeString))
			{
				for (int i = 0; i < 2; i++) {
					bool isSelected = currentCamereTypeString == cameraType[i];
					if (ImGui::Selectable(cameraType[i], isSelected)) {
						currentCamereTypeString = cameraType[i];
						camera.sceneCam.set_camProjType((SceneCamera::CameraProjType)i);
					}
				}
				ImGui::EndCombo();
			}
			if (camera.sceneCam.get_camProjType() == SceneCamera::CameraProjType::orthographic) {
				float size = camera.sceneCam.get_ortho_size();
				float nearClip = camera.sceneCam.get_ortho_nearClip();
				float farClip = camera.sceneCam.get_ortho_farClip();
				ImGui::Text("OrthographicSize : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##size", &size))
					camera.sceneCam.set_ortho_size(size);
				ImGui::Text("NearClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##near", &nearClip))
					camera.sceneCam.set_ortho_nearClip(nearClip);
				ImGui::Text("FarClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##far", &farClip))
					camera.sceneCam.set_ortho_farClip(farClip);
			}
			else {
				float FOV = glm::degrees(camera.sceneCam.get_persp_FOV());
				float nearClip = camera.sceneCam.get_persp_nearClip();
				float farClip = camera.sceneCam.get_persp_farClip();
				ImGui::Text("Perspective FOV : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##FOV", &FOV))
					camera.sceneCam.set_persp_FOV(glm::radians(FOV));
				ImGui::Text("NearClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##near", &nearClip))
					camera.sceneCam.set_persp_nearClip(nearClip);
				ImGui::Text("FarClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##far", &farClip))
					camera.sceneCam.set_persp_farClip(farClip);
			}
			}, 4);

		DrawComponent<ShadingComponent>("Materials", entity, [&](auto& comp) {
			ImGui::ColorEdit4("Color", glm::value_ptr(comp.color));
			}, 1);
	}


	void SceneProperties::drawEntityNode(Entity entity)
	{
		TagComponent& tagComp = entity.getComponent<TagComponent>();
		const bool selected = isSelected(entity);

		// Mid inline rename -- this frame an input box takes the node's place.
		// (So the children of the node being renamed briefly disappear. The existing behavior is kept.)
		if (selected && editMode)
		{
			ImGui::SetKeyboardFocusHere();
			memset(editBuffer, 0, sizeof(editBuffer));
			strcpy_s(editBuffer, sizeof(editBuffer), tagComp.name.c_str());
			if (tagComp.nameEditable) {
				if (ImGui::InputText("##DirectEdit", editBuffer, sizeof(editBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					tagComp.name = std::string(editBuffer);
					editMode = false;
				}
				if (ImGui::IsItemDeactivated() || ImGui::IsKeyPressed(ImGuiKey_Escape))
					editMode = false;
			}
			else {
				editMode = false;
			}
			return;
		}

		// Whether there are children is checked first. Without Leaf, a node with no children still
		// gets an expand arrow, so everything ends up with an empty arrow that opens nothing.
		const std::vector<entt::entity>* children = pm_scene->getChildren(entity);
		const bool hasChildren = children && !children->empty();

		set_vis_color(entity);
		ImGuiTreeNodeFlags flags =
			(selected ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_DefaultOpen;
		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagComp.name.c_str());
		// Reverted right after the label is fully drawn. Left unpopped, children inherit the parent's
		// grey and it looks as if the children were hidden when only the parent was.
		ImGui::PopStyleColor();

		// WARNING: the click test has to be *right after* TreeNodeEx. Pushed past the children,
		//    ImGui's "last item" becomes a child and the wrong node gets selected.
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			clickedOnEntity = true;
			// WARNING: this used to be KeyCtrl alone, toggle only. Now it uses the same table as the
			//    viewport -- which modifier that is is not here; the select group in keymap.yaml decides.
			handleSelectionClick(entity, selectModeFromMods());
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (tagComp.nameEditable) {
				editMode = true;
				if (!isSelected(entity))
					pm_selectedEntities.push_back(entity);
			}
			else { APP_WMS_CORE_WARN("Not editable the name."); }
		}

		// -- Drag & drop ----------------------------------------------
		// These two blocks attach to the "last item" TreeNodeEx created. Pushed past the children
		// they would attach to a child node, so they must stay above the recursion.
		if (ImGui::BeginDragDropSource()) {
			const entt::entity payload = entity.get_entityID();
			ImGui::SetDragDropPayload("WMS_ENTITY", &payload, sizeof(payload));
			ImGui::TextUnformatted(tagComp.name.c_str());   // the preview that follows the cursor
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("WMS_ENTITY")) {
				// WARNING: setParent must not be called here -- this function is mid-recursion and if the
				//    parent/child vectors get torn up, the position being iterated dies. Just record it.
				pm_dragSource = *(const entt::entity*)p->Data;
				pm_dropTarget = entity.get_entityID();
				pm_hasPendingReparent = true;
			}
			ImGui::EndDragDropTarget();
		}

		if (opened && hasChildren) {
			// WARNING: iterate over a copy. If K-d's drag & drop changes the hierarchy during this
			//    traversal, the original vector is reallocated and the iterator dies right there.
			const std::vector<entt::entity> snapshot = *children;
			for (entt::entity child : snapshot) {
				Entity childEntity{ child, pm_scene.get() };
				if (!pm_scene->isValid(childEntity))
					continue;
				drawEntityNode(childEntity);
			}
			ImGui::TreePop();
		}
	}


	bool SceneProperties::onKeyPressed(int keyCode)
	{
		// With no selection this panel has nothing to handle -- pass it on elsewhere.
		if (pm_selectedEntities.empty())
			return false;

		if (KeyMap::isAction("edit.delete", keyCode))         { deleteSelected();          return true; }
		if (KeyMap::isAction("visibility.hide", keyCode))     { setSelectedVisible(false); return true; }
		if (KeyMap::isAction("visibility.show", keyCode))     { setSelectedVisible(true);  return true; }
		// WARNING: parent (P) and unparent (Shift+P) use the same key. Because KeyMap::isAction
		//    requires the modifiers to match *exactly*, P does not swallow Shift+P.
		//    With a partial match the parent above would always win and unparent would be dead.
		if (KeyMap::isAction("hierarchy.parent", keyCode))    { parentSelected();          return true; }
		if (KeyMap::isAction("hierarchy.unparent", keyCode))  { unparentSelected();        return true; }
		return false;
	}

	void SceneProperties::parentSelected()
	{
		// The Maya convention -- pick the children first, pick *the parent last*, then P.
		// So the active (last) item is the parent and everything before it becomes a child.
		// It is the same item the gizmo and the Property editor attach to, so expectations hold.
		if (pm_selectedEntities.size() < 2) {
			APP_WMS_CORE_WARN("parent: at least two must be selected - children first, the parent last.");
			return;
		}

		Entity parent = pm_selectedEntities.back();
		// WARNING: iterate over a copy. setParent does not touch the list when it fails, but
		//    once the selection starts being modified, changing it mid-traversal becomes a problem.
		const std::vector<Entity> children(pm_selectedEntities.begin(), pm_selectedEntities.end() - 1);
		for (const Entity& child : children) {
			if (!pm_scene->isValid(child))
				continue;
			// A cycle (a descendant as the parent) is rejected by setParent, which logs a warning.
			pm_scene->setParent(child, parent);
		}
	}

	void SceneProperties::unparentSelected()
	{
		// Same as Maya's unparent -- pulled out to the root while keeping its world position.
		for (Entity& e : pm_selectedEntities) {
			if (pm_scene->isValid(e))
				pm_scene->removeParent(e);
		}
	}

	void SceneProperties::applyPendingReparent()
	{
		if (!pm_hasPendingReparent)
			return;
		pm_hasPendingReparent = false;

		Entity source{ pm_dragSource, pm_scene.get() };
		if (!pm_scene->isValid(source))
			return;

		// If the dragged item is in the current selection, the whole selection moves (DCC convention).
		// If something outside the selection was dragged, only that one moves.
		std::vector<Entity> movers;
		if (isSelected(source))
			movers = pm_selectedEntities;
		else
			movers.push_back(source);

		// Dropped on empty space means target is null -> unparent to the root
		Entity target = (pm_dropTarget == entt::null)
			? Entity{}
			: Entity{ pm_dropTarget, pm_scene.get() };

		for (Entity& m : movers) {
			if (!pm_scene->isValid(m))
				continue;
			if (target && m == target)
				continue;   // dropped onto itself
			// A cycle is rejected by setParent -- it is not checked again here.
			pm_scene->setParent(m, target);
		}
	}

	void SceneProperties::setSelectedVisible(bool visible)
	{
		for (Entity& e : pm_selectedEntities) {
			if (pm_scene->isValid(e))
				e.getComponent<VisComponent>().visible = visible ? 1 : 0;
		}
	}

	void SceneProperties::deleteSelected()
	{
		// WARNING: deletion takes the whole subtree (the Maya way). If a parent and a child are both
		//    in the selection list, deleting the parent kills the child handle too. The old approach
		//    of erasing while holding an iterator calls getComponent on that dead handle and blows up.
		//    So rather than erasing while iterating, only "what survives" is collected anew -- the dead fall out.
		std::vector<Entity> keep;
		for (Entity& e : pm_selectedEntities) {
			if (!pm_scene->isValid(e))
				continue;   // already gone, swept up by an earlier subtree deletion

			if (e.hasComponent<CameraMatrixComponent>() &&
				e.getComponent<CameraMatrixComponent>().defaultCam) {
				std::cout << "default camera can't be deleted" << std::endl;
				keep.push_back(e);
				continue;
			}
			pm_scene->deletEntity(e);
		}
		pm_selectedEntities = std::move(keep);
	}



	void SceneProperties::set_vis_color(Entity& entity) {
		auto& visComp = entity.getComponent<VisComponent>();
		if (!visComp.visible) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.933f, 0.933f, 0.933f, 1.0f));
	}

}
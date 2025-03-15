#include "pch.h"
#include <imgui/imgui.h>
#include <imGui/imgui_internal.h>
#include "sceneProperty.h"
#include "scene/component.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ranges>

namespace AppWMs {
	template<typename T, typename ui_buildFunc>
	static void DrawComponent(const std::string& label, const Entity& entity, ui_buildFunc uiFunc, const int& componentCount) {
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

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
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
	void SceneProperties::onImGuiRender() {
		ImGui::Begin("Scene properties");

		// Check if the left mouse button is clicked in the window
		bool clickedInsidePanel = ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		
		clickedOnEntity = false;
		// Loop through all entities to check if any entity was clicked
		for (auto& e_id : pm_scene->pm_register.view<entt::entity>()) {
			Entity entity{ e_id, pm_scene.get() };
			drawModel(entity);

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				clickedOnEntity = true;
				if (ImGui::GetIO().KeyCtrl) {
					if (pm_selectedEntities.find(entity) == pm_selectedEntities.end()) {
						pm_selectedEntities.insert(entity);
					}
					else { pm_selectedEntities.erase(entity); }
				}
				else {
					pm_selectedEntities.clear();
					pm_selectedEntities.insert(entity);
				}
			}
		}
		// set function to set visbility as the false or true.
		set_vis_ctrl();

		if (clickedInsidePanel && !clickedOnEntity) {
			// Only deselect if there were selected entities
			if (!pm_selectedEntities.empty()) {
				pm_selectedEntities.clear();
				std::cout << "Deselected all entities!" << std::endl;
			}
		}
		ImGui::End();


		ImGui::Begin("Property editor");
		// If there are selected entities, show their properties
		if (!pm_selectedEntities.empty()) {
			showPropertyComponents(*pm_selectedEntities.begin());
			for (const Entity& selectedEntity : pm_selectedEntities) {
				set_deleteEntity(selectedEntity);
			}
		}
		ImGui::End();
	}




	void SceneProperties::setScene(const Ref<Scene>& scene)
	{
		pm_scene = scene;
		pm_selectedEntities.clear();
	}


	void SceneProperties::showPropertyComponents(const Entity& entity)
	{
		if (entity.hasComponent<TagComponent>()) {
			char buffer[256];
			auto& tagComp = entity.getComponent<TagComponent>();
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tagComp.name.c_str());
			if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				tagComp.name = std::string(buffer); // Update item name when the input is changed
			}
			ImGui::Separator();
		}

		DrawComponent<TransformComponent>("Transform", entity, [](auto& transComp) {
			DrawVec3("Translate", transComp.translate);
			DrawVec3("Rotate", transComp.rotate);
			DrawVec3("Scale", transComp.scale, 1.0f);
			}, 3);
		DrawComponent<VisComponent>("Visibility", entity, [](auto& visComp) {
			DrawBool(visComp.visible);
			}, 1);

		DrawComponent<CameraMatrixComponent>("Camera", entity, [](auto& camera) {
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
				if (ImGui::DragFloat("##size", &size, 0.001f))
					camera.sceneCam.set_ortho_size(size);
				ImGui::Text("NearClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##near", &nearClip, 0.001f))
					camera.sceneCam.set_ortho_nearClip(nearClip);
				ImGui::Text("FarClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##far", &farClip, 0.001f))
					camera.sceneCam.set_ortho_farClip(farClip);
			}
			else {
				float FOV = glm::degrees(camera.sceneCam.get_persp_FOV());
				float nearClip = camera.sceneCam.get_persp_nearClip();
				float farClip = camera.sceneCam.get_persp_farClip();
				ImGui::Text("Perspective FOV : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##FOV", &FOV, 0.001f))
					camera.sceneCam.set_persp_FOV(glm::radians(FOV));
				ImGui::Text("NearClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##near", &nearClip, 0.001f))
					camera.sceneCam.set_persp_nearClip(nearClip);
				ImGui::Text("FarClip : ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##far", &farClip, 0.001f))
					camera.sceneCam.set_persp_farClip(farClip);
			}
			}, 4);

		DrawComponent<ShadingComponent>("Materials", entity, [](auto& comp) {
			ImGui::ColorEdit4("Color", glm::value_ptr(comp.color));
			}, 1);
	}


	void SceneProperties::drawModel(Entity& entity)
	{
		TagComponent& tagComp = entity.getComponent<TagComponent>();

		bool isSelected = (pm_selectedEntities.find(entity) != pm_selectedEntities.end());
		if (!isSelected || !editMode)
		{
			set_vis_color(entity);
			ImGuiTreeNodeFlags flags =
				(isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagComp.name.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (tagComp.nameEditable)
				{
					editMode = true;
					pm_selectedEntities.insert(entity);
				}
				else { APP_WMS_CORE_WARN("Not editable the name."); }
			}

			if (opened)
				ImGui::TreePop();
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::SetKeyboardFocusHere();
			memset(editBuffer, 0, sizeof(editBuffer));
			strcpy_s(editBuffer, sizeof(editBuffer), tagComp.name.c_str());
			if (entity.getComponent<TagComponent>().nameEditable) {
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
		}
	}


	void SceneProperties::set_deleteEntity(const Entity& entity)
	{
		if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Delete, false)) {
			if (entity.hasComponent<CameraMatrixComponent>()) {
				if (!entity.getComponent<CameraMatrixComponent>().defaultCam) {
					pm_scene->deletEntity(entity);
				}
				else {
					std::cout << "default camera can't be deleted" << std::endl;
				}
			}
			else {
				pm_scene->deletEntity(entity);
			}
		}
	}



	void SceneProperties::set_vis_color(Entity& entity) {
		auto& visComp = entity.getComponent<VisComponent>();
		if (!visComp.visible) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.933f, 0.933f, 0.933f, 1.0f));
	}

	void SceneProperties::set_vis_ctrl()
	{
		if (pm_selectedEntities.size() > 0){
			for (auto& e : pm_selectedEntities) {
				auto& visComp = e.getComponent<VisComponent>();
				if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_H)) {
					std::cout << "Shift+H Pressed!" << std::endl;
					visComp.visible = false; // Ctrl+H hides
				}
				if (ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_H)) {
					std::cout << "Ctrl+H Pressed!" << std::endl;
					visComp.visible = true;  // Shift+H shows
				}
			}

		}
	}
}
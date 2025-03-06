#include "pch.h"
#include <imgui/imgui.h>
#include <imGui/imgui_internal.h>
#include "sceneProperty.h"
#include "scene/component.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ranges>

namespace AppWMs {

	static void DrawVec3(const std::string& label, glm::vec3& values, float resetValue=0.0f, float columnWidth=100.0f) {
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		int i = 0;

		std::array<const char*,3> translate_comp { "X", "Y", "Z"};
		std::array<float*, 3> values_comp { &values.x, &values.y, &values.z };
		std::array<ImVec4, 3> RGB{ ImVec4(0.78f, 0.25f, 0.15f, 1.0f),
			ImVec4(0.3f, 0.78f, 0.2f, 1.0f),
			ImVec4(0.3f, 0.4f, 0.9f, 1.0f) };
		auto keyIt = translate_comp.begin();
		auto valueIt = values_comp.begin();
		auto colorIt = RGB.begin();

		while (keyIt != translate_comp.end() && valueIt != values_comp.end()) {
			ImGui::PushStyleColor(ImGuiCol_Text, *colorIt);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.25f, .25f, .25f, 1.0f));

			if (ImGui::Button(*keyIt, buttonSize)) 
				**valueIt = resetValue;
			ImGui::PopStyleColor(2);

			char hash_label[32];
			snprintf(hash_label, sizeof(hash_label), "##%s", *keyIt);
			ImGui::SameLine();
			ImGui::DragFloat(hash_label , *valueIt, 0.01f);
			ImGui::PopItemWidth();
			if (i <2)
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

	SceneProperties::SceneProperties(const Ref<Scene>& scene):pm_scene(scene)
	{

	}



	void SceneProperties::onImGuiRender()
	{
		ImGui::Begin("Scene properties");
		for (auto& e_id : pm_scene->pm_register.view<entt::entity>()) {
			Entity entity{ e_id, pm_scene.get() };
			drawModel(entity);
		}
		// deselect
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			pm_selectedNode = {};
		}
		ImGui::End();

		ImGui::Begin("Property editor");
		if (pm_selectedNode)
			showPropertyComponents(pm_selectedNode);
		ImGui::End();
	}

	void SceneProperties::setScene(const Ref<Scene>& scene)
	{
		pm_scene = scene;
	}


	void SceneProperties::showPropertyComponents(Entity& entity)
	{
		if (entity.hasComponent<TagComponent>()) {
			char buffer[256];
			auto& tagComp = entity.getComponent<TagComponent>();
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tagComp.name.c_str());
			if ( ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue) )
			{
				tagComp.name = std::string(buffer); // Update item name when the input is changed
			}
			ImGui::Separator();
		}

		if (entity.hasComponent<TransformComponent>()) {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.14f, 0.16f, 0.14f, 1.0f));
			if (ImGui::TreeNodeEx("Translate", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
				// Child frame for the DragFloat3
				ImGui::BeginChild("TranslateGroup", ImVec2(0, 70), ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY,
					ImGuiWindowFlags_NoScrollbar);
				auto& transComp = entity.getComponent<TransformComponent>();
				DrawVec3("Translate", transComp.translate);
				DrawVec3("Rotate", transComp.rotate);
				DrawVec3("Scale", transComp.scale, 1.0f);
				//ImGui::DragFloat3("trans", glm::value_ptr(translate), 0.01f);
				ImGui::EndChild();
				ImGui::TreePop();  // Pop the collapsible group
			}
			ImGui::PopStyleColor();
		}

		if (entity.hasComponent<CameraMatrixComponent>()) {
			auto& camera = entity.getComponent<CameraMatrixComponent>();
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.14f, 0.16f, 0.14f, 1.0f));
			if (ImGui::TreeNodeEx("Camera type", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) 
			{
				// Child frame for the DragFloat3
				ImGui::BeginChild("CameraGroup", ImVec2(320, 90), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY,
					ImGuiWindowFlags_NoScrollbar);
				const char* cameraType[] = { "perspective", "orthographic" };
				const char* currentCamereTypeString = cameraType[(int)camera.sceneCam.get_camProjType()];
				if (ImGui::BeginCombo("##Camera Type", currentCamereTypeString)) 
				{
					for (int i=0; i < 2; i++) {
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
					if (ImGui::DragFloat("##size", &FOV))
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
				ImGui::EndChild();
				ImGui::TreePop();  // Pop the collapsible group
			}
			ImGui::PopStyleColor();
		}

		if (entity.hasComponent<ShadingComponent>()) {
			auto& color = entity.getComponent<ShadingComponent>().color;
			ImGui::ColorEdit4(" : Color", glm::value_ptr(color));
			ImGui::Separator();
		}
	}


	void SceneProperties::drawModel(Entity& entity)
	{
		TagComponent& tagComp = entity.getComponent<TagComponent>();

		bool isSelected = (pm_selectedNode == entity);

		if (!isSelected || !editMode)
		{
			ImGuiTreeNodeFlags flags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagComp.name.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				editMode = true;
				pm_selectedNode = entity;
			}
			else if (ImGui::IsItemClicked()) {
				pm_selectedNode = entity;

			}
			if (opened)
				ImGui::TreePop();
		}
		else
		{
			ImGui::SetKeyboardFocusHere();
			memset(editBuffer, 0, sizeof(editBuffer));
			strcpy_s(editBuffer, sizeof(editBuffer), tagComp.name.c_str());
			if (ImGui::InputText("##DirectEdit", editBuffer, sizeof(editBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				tagComp.name = std::string(editBuffer);
				editMode = false;
			}

			if (ImGui::IsItemDeactivated() || ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				editMode = false;
			}
		}
	}

}
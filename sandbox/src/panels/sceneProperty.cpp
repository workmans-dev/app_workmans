#include "pch.h"
#include <imgui/imgui.h>
#include "sceneProperty.h"
#include "scene/component.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace AppWMs {



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
				ImGui::BeginChild("TranslateGroup", ImVec2(0, 20), ImGuiChildFlags_AutoResizeX|ImGuiChildFlags_AutoResizeY,
					ImGuiWindowFlags_NoScrollbar);
				auto& transform = entity.getComponent<TransformComponent>().transform;
				ImGui::DragFloat3("##Translate", glm::value_ptr(transform[3]), 0.1f);
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
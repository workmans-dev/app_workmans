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
	// 쿼터니언 편집기. DrawVec3와 달리 축 색을 안 쓴다 —
	// w/x/y/z는 축이 아니라 4차원 성분이라 RGB=XYZ 관례를 붙이면 거짓말이 된다.
	//
	// ⚠️ 여기서 성분을 직접 드래그하면 길이가 1을 벗어난다. 정규화는
	//    getLocalMatrix()가 읽는 시점에 하므로(safeNormalizeQuat) 화면은 안 깨지지만,
	//    표시값과 실제 적용값이 갈릴 수 있다. 정상적인 조작 경로는 기즈모다.
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
			// 버튼 = 항등 쿼터니언으로 리셋 (W만 1, 나머지 0)
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
		// 마지막에 고른 것이 활성이다 (마야 규약).
		return pm_selectedEntities.empty() ? nullptr : &pm_selectedEntities.back();
	}

	void SceneProperties::handleSelectionClick(const Entity& e, bool additive)
	{
		if (!additive) {
			pm_selectedEntities.clear();
			pm_selectedEntities.push_back(e);
			return;
		}

		auto it = std::find(pm_selectedEntities.begin(), pm_selectedEntities.end(), e);
		if (it == pm_selectedEntities.end()) {
			// ⚠️ 항상 **뒤에** 붙인다. 이 위치가 곧 선택 순서고,
			//    마지막 원소가 활성(= parent의 부모)이 된다.
			pm_selectedEntities.push_back(e);
		}
		else {
			// 이미 골라져 있던 걸 Ctrl+클릭 → 해제.
			// erase로 앞당겨져도 나머지의 상대 순서는 유지된다.
			pm_selectedEntities.erase(it);
		}
	}

	void SceneProperties::pruneInvalidSelection()
	{
		// 서브트리 삭제(마야 방식) 때문에 내가 안 지운 엔티티도 사라질 수 있다 —
		// 부모를 지우면 자손이 함께 죽는다. 죽은 핸들로 getComponent를 부르면 그대로 터진다.
		pm_selectedEntities.erase(
			std::remove_if(pm_selectedEntities.begin(), pm_selectedEntities.end(),
				[this](const Entity& e) { return !pm_scene->isValid(e); }),
			pm_selectedEntities.end());
	}

	void SceneProperties::onImGuiRender() {
		ImGui::Begin("Scene properties");

		// 그리기 전에 한 번 턴다. 지난 프레임의 삭제로 죽은 핸들이 남아 있을 수 있다.
		pruneInvalidSelection();

		// Check if the left mouse button is clicked in the window
		bool clickedInsidePanel = ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

		clickedOnEntity = false;
		// ⚠️ view<TransformComponent> — **오브젝트만** 훑는다.
		//    아웃라이너는 "씬에 놓인 물건"의 목록이다. 모디파이어처럼 위치가 없는
		//    엔티티는 여기 뜨면 안 된다(블렌더도 모디파이어를 아웃라이너 항목으로
		//    안 보여주고, 마야는 기본이 "Show DAG objects only"다). 그것들은
		//    자기 탭에서 다뤄지고, 이 순회가 컴포넌트 구성만으로 알아서 걸러낸다.
		//    덤으로 아래 set_vis_color가 VisComponent를 무조건 가정해도 안전해진다.
		//
		// 루트만 훑는다. 자식은 각 노드가 자기 안에서 재귀로 그린다 —
		// 여기서 전부 훑으면 자식이 부모 밑에 한 번, 최상위에 또 한 번 나온다.
		for (auto e_id : pm_scene->pm_register.view<TransformComponent>()) {
			Entity entity{ e_id, pm_scene.get() };
			if (pm_scene->getParent(entity))
				continue;
			drawEntityNode(entity);
		}
		// 트리 아래 남은 빈 공간도 드롭 타겟이다 — 여기 놓으면 루트로 뺀다(unparent).
		// 마야 아웃라이너와 같은 동작이고, 계층에서 빼는 데 단축키 말고 마우스 경로도 준다.
		const ImVec2 remaining = ImGui::GetContentRegionAvail();
		if (remaining.x > 0.0f && remaining.y > 0.0f) {
			ImGui::Dummy(remaining);
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("WMS_ENTITY")) {
					pm_dragSource = *(const entt::entity*)p->Data;
					pm_dropTarget = entt::null;      // 부모 없음 = 루트로
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

		// 트리 순회가 완전히 끝난 뒤에 계층을 바꾼다. 위 주석의 이유 그대로다.
		applyPendingReparent();

		ImGui::Begin("Property editor");
		// 마지막에 고른 것의 속성을 보여준다 (마야 어트리뷰트 에디터와 같은 규칙).
		if (Entity* active = get_activeEntity())
			showPropertyComponents(*active);
		ImGui::End();

	}

	void SceneProperties::setScene(const Ref<Scene>& scene)
	{
		pm_scene = scene;
		pm_selectedEntities.clear();
	}


	// 탭 바 호스트.
	//
	// ⚠️ 탭을 나눈 건 화면 정리가 아니라 **경계를 코드로 못박기 위해서**다.
	//    Object(이 물건 자체)와 Modifier(이 물건에 거는 연산)는 서로 다른 종류의
	//    엔티티를 다룬다 — 오브젝트는 위치가 있고 모디파이어는 없다.
	//    한 패널에 섞어두면 "add component"로 모디파이어를 만드는 흐름이 생기고,
	//    그 순간 둘의 구분이 무너진다. 지금 벽을 세워둔다.
	//
	// Data 탭(형상 자체)은 Ref<Mesh>가 생길 때 붙는다 — 지금 만들면 담을 게 없다.
	void SceneProperties::showPropertyComponents( Entity& entity)
	{
		// 이름은 탭 밖에 둔다. 어느 탭을 보든 "무엇을 보고 있는지"는 항상 보여야 한다.
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
		// 🚧 아직 모디파이어 실물이 없다. 이 탭은 경계를 먼저 세워둔 자리다.
		//    미러/스킨 등은 **위치 없는 엔티티**(Scene::createEntity)로 만들어져
		//    이 목록에 뜬다. 오브젝트 엔티티에 컴포넌트로 얹지 않는다 —
		//    그래야 종류별 연속 순회(view<MirrorComponent> 등)로 평가할 수 있다.
		//    블렌더처럼 오브젝트마다 힙에 매달린 리스트를 타면 그 이점이 사라진다.
		// ⚠️ 화면 문자열은 ASCII로 쓴다. 폰트가 Rubik(라틴 전용)이고 글리프 범위도
		//    기본값이라 한글은 빈칸으로 나온다. 한글 UI를 하려면 한글 폰트 +
		//    GetGlyphRangesKorean()을 imGuiLayer의 폰트 로드에 붙여야 한다. (미해결)
		ImGui::TextDisabled("No modifiers.");
		ImGui::Spacing();
		ImGui::TextWrapped("Mirror / Skin / Subdiv will stack here. (not implemented)");
	}

	void SceneProperties::drawObjectTab(Entity& entity)
	{
		DrawComponent<TransformComponent>("Transform", entity, [&](auto& transComp) {
			DrawVec3("Translate", transComp.translate);

			// 회전 모드 — 블렌더 방식. 고르는 순간 표현만 바뀌고 회전 자체는 유지된다
			// (변환은 setRotationMode 안에서 일어난다).
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
						ImGui::Separator();   // 오일러 6종과 쿼터니언은 성격이 다르다
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

		// 인라인 이름 편집 중 — 이 프레임엔 노드 대신 입력창이 그 자리를 차지한다.
		// (그래서 편집 중인 노드의 자식은 잠깐 안 보인다. 기존 동작 그대로 둔다)
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

		// 자식 유무를 먼저 본다. Leaf를 안 주면 자식 없는 노드에도 펼침 화살표가
		// 생겨서, 눌러도 아무것도 안 나오는 빈 화살표가 전부에 붙는다.
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
		// 라벨을 다 그린 직후에 되돌린다. 여기서 안 풀면 자식이 부모의 회색을 물려받아
		// 부모만 숨겼는데 자식도 숨겨진 것처럼 보인다.
		ImGui::PopStyleColor();

		// ⚠️ 클릭 판정은 TreeNodeEx **바로 뒤**여야 한다. 자식을 그린 뒤로 밀면
		//    ImGui의 "마지막 아이템"이 자식으로 바뀌어 엉뚱한 노드가 선택된다.
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			clickedOnEntity = true;
			handleSelectionClick(entity, ImGui::GetIO().KeyCtrl);
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (tagComp.nameEditable) {
				editMode = true;
				if (!isSelected(entity))
					pm_selectedEntities.push_back(entity);
			}
			else { APP_WMS_CORE_WARN("Not editable the name."); }
		}

		// ── 드래그&드롭 ─────────────────────────────────────────────
		// 이 두 블록은 TreeNodeEx가 만든 "마지막 아이템"에 붙는다. 자식을 그린 뒤로
		// 밀면 자식 노드에 붙어버리므로 반드시 재귀보다 위에 있어야 한다.
		if (ImGui::BeginDragDropSource()) {
			const entt::entity payload = entity.get_entityID();
			ImGui::SetDragDropPayload("WMS_ENTITY", &payload, sizeof(payload));
			ImGui::TextUnformatted(tagComp.name.c_str());   // 커서를 따라다니는 미리보기
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("WMS_ENTITY")) {
				// ⚠️ 여기서 setParent를 부르면 안 된다 — 지금 이 함수가 재귀 중이고
				//    부모/자식 vector가 갈아엎히면 순회하던 자리가 죽는다. 적어만 둔다.
				pm_dragSource = *(const entt::entity*)p->Data;
				pm_dropTarget = entity.get_entityID();
				pm_hasPendingReparent = true;
			}
			ImGui::EndDragDropTarget();
		}

		if (opened && hasChildren) {
			// ⚠️ 복사본으로 돈다. K-d의 드래그&드롭이 이 순회 도중에 계층을 바꾸면
			//    원본 vector가 재할당되어 반복자가 그 자리에서 죽는다.
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
		// 선택이 없으면 이 패널이 처리할 게 없다 — 다른 데로 흘려보낸다.
		if (pm_selectedEntities.empty())
			return false;

		if (KeyMap::isAction("edit.delete", keyCode))         { deleteSelected();          return true; }
		if (KeyMap::isAction("visibility.hide", keyCode))     { setSelectedVisible(false); return true; }
		if (KeyMap::isAction("visibility.show", keyCode))     { setSelectedVisible(true);  return true; }
		// ⚠️ parent(P)와 unparent(Shift+P)가 같은 키를 쓴다. KeyMap::isAction이
		//    조합키 **정확히 일치**를 요구하기 때문에 P가 Shift+P를 삼키지 않는다.
		//    부분 일치였으면 위에 있는 parent가 항상 먼저 걸려 unparent가 죽는다.
		if (KeyMap::isAction("hierarchy.parent", keyCode))    { parentSelected();          return true; }
		if (KeyMap::isAction("hierarchy.unparent", keyCode))  { unparentSelected();        return true; }
		return false;
	}

	void SceneProperties::parentSelected()
	{
		// 마야 규약 — 자식들을 먼저 고르고 **부모를 마지막에** 고른 뒤 P.
		// 그래서 활성(마지막) 항목이 부모고, 그 앞의 전부가 자식이 된다.
		// 기즈모·Property editor가 붙는 대상과 같은 항목이라 예측이 어긋나지 않는다.
		if (pm_selectedEntities.size() < 2) {
			APP_WMS_CORE_WARN("parent: 최소 2개를 골라야 한다 — 자식들을 먼저, 부모를 마지막에.");
			return;
		}

		Entity parent = pm_selectedEntities.back();
		// ⚠️ 복사본으로 돈다. setParent는 실패하면 목록을 안 건드리지만,
		//    앞으로 선택을 손대게 되면 순회 중 변경이 바로 문제가 된다.
		const std::vector<Entity> children(pm_selectedEntities.begin(), pm_selectedEntities.end() - 1);
		for (const Entity& child : children) {
			if (!pm_scene->isValid(child))
				continue;
			// 순환(자손을 부모로)은 setParent가 거부하고 경고를 찍는다.
			pm_scene->setParent(child, parent);
		}
	}

	void SceneProperties::unparentSelected()
	{
		// 마야 unparent와 같다 — 월드 위치를 유지한 채 루트로 뺀다.
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

		// 끈 항목이 현재 선택에 들어 있으면 선택 전체를 옮긴다 (DCC 관례).
		// 선택 밖의 것을 끌었으면 그 하나만 옮긴다.
		std::vector<Entity> movers;
		if (isSelected(source))
			movers = pm_selectedEntities;
		else
			movers.push_back(source);

		// 빈 공간에 놓았으면 target이 null → 루트로 빼기(unparent)
		Entity target = (pm_dropTarget == entt::null)
			? Entity{}
			: Entity{ pm_dropTarget, pm_scene.get() };

		for (Entity& m : movers) {
			if (!pm_scene->isValid(m))
				continue;
			if (target && m == target)
				continue;   // 자기 자신 위에 놓은 경우
			// 순환은 setParent가 거부한다 — 여기서 또 검사하지 않는다.
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
		// ⚠️ 삭제는 서브트리 통째다(마야 방식). 선택 목록에 부모와 자식이 같이 들어
		//    있으면 부모를 지우는 순간 자식 핸들도 죽는다. 반복자를 들고 지우던 예전
		//    방식은 그 죽은 핸들에 getComponent를 불러 터진다.
		//    그래서 지우면서 도는 대신 "살릴 것"만 새로 모은다 — 죽은 건 자연히 빠진다.
		std::vector<Entity> keep;
		for (Entity& e : pm_selectedEntities) {
			if (!pm_scene->isValid(e))
				continue;   // 앞선 서브트리 삭제에 휩쓸려 이미 사라졌다

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
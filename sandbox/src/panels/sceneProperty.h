#pragma once

#include "core/core.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include <vector>


namespace AppWMs {

	class SceneProperties {
	public:
		SceneProperties() = default;
		SceneProperties(const Ref<Scene>& scene);
		void onImGuiRender();
		void setScene(const Ref<Scene>& scene);
		// Property editor 본체 — 이름 + 탭 바.
		// 탭은 화면 정리가 아니라 **경계**다. Object(물건 자체) / Modifier(거는 연산)는
		// 서로 다른 종류의 엔티티를 다루므로 섞이면 안 된다.
		void showPropertyComponents( Entity& entity);
		// 한 노드를 그리고, 펼쳐져 있으면 자식까지 재귀로 그린다.
		// ⚠️ 값으로 받는다 — 재귀가 자식 Entity를 임시로 만들기 때문에 참조로는 못 묶는다.
		void drawEntityNode(Entity entity);
		void set_vis_color(Entity& entity);

		// 키 이벤트 경로. 처리했으면 true.
		//
		// ⚠️ 예전엔 ImGui::IsKeyPressed로 매 프레임 직접 물었다. 그러면 (1) 키가
		//    keymap.yaml이 아니라 코드에 박히고, (2) ImGuiKey 열거형이라 우리
		//    키코드 체계와 또 갈라진다. 이제 다른 단축키와 같은 이벤트 경로를 탄다.
		bool onKeyPressed(int keyCode);

		// ⚠️ set이 아니라 vector다 — **선택한 순서가 정보**이기 때문이다.
		//    마야 규약: 자식들을 먼저 고르고 부모를 마지막에 고른 뒤 P.
		//    std::set은 엔티티 id 순으로 정렬해버려서 그 순서를 통째로 버린다.
		//    (그래서 EntityComparator는 이제 여기서 안 쓴다)
		std::vector<Entity>& get_selectedEntities() { return pm_selectedEntities; }

		// 선택된 것 중 **활성** 오브젝트 = 마지막에 고른 것.
		// 마야에서 매니퓰레이터와 어트리뷰트 에디터가 붙는 대상이 이것이고,
		// parent의 부모가 되는 대상도 이것이다. 하나로 통일해야 헷갈리지 않는다.
		Entity* get_activeEntity();

	private:
		// 선택 조작 — 클릭 처리부가 트리 순회 안으로 들어가도(K-b) 그대로 재사용된다.
		void drawNameField(Entity& entity);
		void drawObjectTab(Entity& entity);     // 이 물건 자체 — Transform / Visibility / Camera / Material
		void drawModifierTab(Entity& entity);   // 이 물건에 거는 연산 — Mirror / Skin ... (🚧 미구현)

		bool isSelected(const Entity& e) const;
		void handleSelectionClick(const Entity& e, bool additive);
		// 삭제로 사라진 핸들을 선택 목록에서 걷어낸다.
		void pruneInvalidSelection();
		void deleteSelected();
		void setSelectedVisible(bool visible);

		// 계층 조작 — 마야 규약(자식들 먼저, 부모를 마지막에 고른 뒤 P)
		void parentSelected();
		void unparentSelected();

		// ⚠️ 드롭을 그 자리에서 처리하면 안 된다. drawEntityNode가 재귀 중이고,
		//    setParent가 부모/자식 vector를 갈아엎으면 순회하던 자리가 그대로 죽는다.
		//    요청만 적어두고 트리를 다 그린 뒤에 적용한다.
		void applyPendingReparent();

	private:
		Ref<Scene> pm_scene;
		std::vector<Entity> pm_selectedEntities;

		// 지연 처리용 드롭 요청. target == entt::null 이면 루트로 빼기(unparent).
		bool         pm_hasPendingReparent = false;
		entt::entity pm_dragSource{ entt::null };
		entt::entity pm_dropTarget{ entt::null };

	private:
		bool editMode = false;
		char editBuffer[256] = "";
		bool clickedOnEntity = false;
	};

}
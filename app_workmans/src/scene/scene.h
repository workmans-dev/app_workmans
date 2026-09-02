#pragma once
#include "core/timer.h"
#include "core/core.h"
#include "renderer/texture.h"
#include "entt.hpp"
#include "renderer/renderer.h"
#include "renderer/camera/sceneCamera.h"
#include "events/event.h"
#include <glm/gtc/matrix_transform.hpp>



namespace AppWMs {
	class Entity;
	class Scene {
	public:
		Scene();
		~Scene();
		// ── 엔티티를 만드는 두 갈래 ───────────────────────────────────────
		//
		// ⚠️ "이게 무슨 종류인가"를 TagComponent의 enum 같은 걸로 들지 않는다.
		//    ECS에선 **어떤 컴포넌트를 갖고 있는가**가 그 질문의 답이다.
		//    enum으로 들면 모든 순회에 if (type == ...) 분기가 붙어 연속 순회의
		//    이점이 사라진다 — 형상을 mesh/curve/nurbs로 나눈 것과 같은 논리다.
		//
		// createEntity : 순수 할당. 이름만 붙는다.
		//                위치가 없는 것들 — 모디파이어(미러/스킨), 디포머 같은
		//                "계산하는" 엔티티가 여기로 온다. transform이 없으니
		//                view<TransformComponent>가 알아서 걸러낸다.
		//
		// createObject : 씬 오브젝트. 공간에 위치가 있고 화면에 나온다.
		//                Transform + WorldTransform + Vis 를 얹는다.
		//
		// 마야는 DAG 노드(위치 있음) / DG 노드(계산)로, 블렌더는 Object / Modifier로
		// 같은 선을 긋는다. 둘 다 "위치가 있느냐"가 기준이다.
		Entity createEntity(const std::string& name = std::string());
		Entity createObject(const std::string& name = std::string());
		// ⚠️ 마야 방식 — 서브트리를 통째로 지운다. 부모를 지우면 자손도 사라진다.
		//    예외는 기본 카메라(perspective/top/front/side)뿐이다. 아래 구현 주석 참고.
		void deletEntity( Entity& entity);
		const entt::registry& get_reg() { return pm_register; }
		// 이 핸들이 아직 살아 있나. 서브트리 삭제(마야 방식) 때문에 내가 안 지운
		// 엔티티도 사라질 수 있다 — 부모를 지우면 자손이 함께 죽는다.
		// 그래서 Entity를 목록으로 들고 있는 쪽(선택 목록 등)은 이걸로 걸러야 한다.
		bool isValid(const Entity& e) const;
		void onUpdate(Timer ts);
		void event(Event& e);
		void onAttach();
		void resizeView(uint32_t w, uint32_t h);
		void set_block_onUpdate(bool& fact) { blocked = fact; }
		void add_camMat_entt(const std::string& entt_tagName, SceneCamera::CameraProjType type,
			bool defaultCam);
		Entity* get_camera_primary();
	public:
		std::vector<std::string>& get_cameraNames(){ return pm_cameraViewArray;}
		std::vector<Entity>& get_camEntities() { return pm_cam_entities; }

	public:
		// ── 계층 ───────────────────────────────────────────────────────────
		//
		// 월드 행렬을 프레임당 한 번 루트→자손 순으로 채운다.
		// onUpdate 최상단에서 부른다. 이 호출 뒤로는 WorldTransformComponent가
		// 이번 프레임의 진실이다.
		//
		// 왜 조회 시점 lazy 계산이 아니라 top-down 1회인가:
		// lazy는 조회마다 부모 체인을 depth만큼 거슬러 올라간다. 본이 깊어지면
		// 엔티티 하나 읽는 데 depth번 재귀가 붙는다. top-down은 정확히 O(N)이고
		// 부모의 월드가 이미 손에 있는 상태로 자식에 내려가므로 재방문이 없다.
		void updateTransforms();

		// child를 parent 밑으로 옮긴다. parent가 빈 Entity면 루트로 뺀다.
		//
		// keepWorldTransform=true면 화면상 위치가 그대로 유지되도록 child의 로컬을
		// 보정한다 (마야 parent / 블렌더 Ctrl+P의 관례). false면 로컬을 그대로 두어
		// 부모 변환만큼 튄다.
		//
		// 실패(자기 자신 / 자손을 부모로 지정 = 순환)하면 false를 돌려주고
		// 아무것도 바꾸지 않는다.
		bool setParent(Entity child, Entity parent, bool keepWorldTransform = true);
		void removeParent(Entity child, bool keepWorldTransform = true);
		// 부모가 없으면 빈 Entity(operator bool == false)를 돌려준다.
		Entity getParent(Entity child);
		// 자식 목록. 계층에 안 엮인 엔티티면 nullptr.
		// ⚠️ 돌려주는 포인터는 setParent/삭제가 벡터를 건드리면 죽는다.
		//    순회 중에 계층을 바꿀 거면 복사본을 떠서 돌 것.
		const std::vector<entt::entity>* getChildren(Entity e) const;
		// updateTransforms()가 이번 프레임에 채운 값. 로컬을 원하면
		// TransformComponent::getLocalMatrix()를 직접 쓴다.
		const glm::mat4& getWorldMatrix(Entity e);

		// 월드 행렬을 직접 지정한다. 부모 밑에서 그 월드가 나오도록 로컬을 역산해 넣는다.
		//     world = parentWorld × local   →   local = inverse(parentWorld) × world
		//
		// 기즈모(월드로 조작)와 setParent(붙여도 안 움직이게 보정)가 같은 식을 쓴다.
		// 두 곳에 각자 적으면 한쪽만 고쳤을 때 "기즈모는 맞는데 부모지정만 튀는"
		// 식으로 갈라지므로 여기 하나만 둔다.
		void setWorldMatrix(Entity e, const glm::mat4& world);

	private:
		// entt::entity : enum class
		entt::registry pm_register;
		// ⚠️ 지오메트리는 씬이 공유본으로 들지 않는다. 엔티티마다 MeshComponent가
		//    자기 것을 소유해야 두 오브젝트를 따로 편집할 수 있다.
		Ref<Texture2D> pm_checker_tex_ptr, pm_icon_tex_ptr;
		
		const glm::mat4& scale_down = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
		const glm::mat4& scale_up = glm::scale(glm::mat4(1.0f), glm::vec3(10.f));
		// glm::vec3(0.0f,0.0f, 0.5f)[z] is for the forwarding the image
		const glm::mat4& transform_icon = glm::translate(glm::mat4(1.0f),
			glm::vec3(0.0f, 0.0f, 0.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
		bool blocked = false;
		friend class Entity;
		friend class Serialize;
		friend class SceneProperties;

		std::vector<std::string> pm_cameraViewArray;
		std::vector<Entity> pm_cam_entities;

	private:
		uint32_t pm_viewport_w, pm_viewport_h;
		template<typename T>
		void componentAdded(Entity& entity, T& comp);

	private:
		// 계층 내부 구현. Entity가 아니라 entt::entity를 받는 이유 —
		// 재귀 중에 registry를 건드리므로 얇은 핸들 하나만 들고 도는 게 안전하다.
		void updateWorldTransform(entt::entity e, const glm::mat4* parentWorld, uint32_t parentVersion);
		void destroySubtree(entt::entity e);
		void detachFromParent(entt::entity e);
		void removeFromCameraLists(entt::entity e);
		bool isDescendantOf(entt::entity maybeDescendant, entt::entity ancestor) const;
		// 공간에 있는 엔티티인가(= createObject로 만들어졌나).
		// 위치 없는 엔티티에 트랜스폼 연산을 걸면 그대로 터지므로 그 앞을 막는다.
		bool isSpatial(entt::entity e) const;


	};

}
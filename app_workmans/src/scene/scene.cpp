#include "pch.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include <glm/glm.hpp>
// 씬은 거의 모든 컴포넌트 타입을 다룬다(순회 + componentAdded 특수화) → 우산이 정직하다
#include "scene/components/components.h"
#include "renderer/geometry/cube.h"


namespace AppWMs {

	// ─────────────────────────────────────────────────────────────
	//  componentAdded 훅 — 컴포넌트가 엔티티에 붙는 순간 씬이 개입할 자리
	//
	//  ⚠️ 이 특수화들은 반드시 첫 사용(onAttach → addComponent)보다 위에 있어야 한다.
	//     명시적 특수화가 암시적 인스턴스화 이후에 나오면 ill-formed다.
	//     예전엔 파일 맨 아래 있었는데, addComponent의 반환 타입이 T&라 MSVC가
	//     본체 인스턴스화를 미뤄줘서 우연히 통과했다. decltype(auto)로 바꾸자마자
	//     (반환 타입을 알려면 본체를 즉시 인스턴스화해야 해서) 드러났다.
	// ─────────────────────────────────────────────────────────────
	template<typename T>
	void Scene::componentAdded(Entity& entity, T& comp) {
		static_assert(false);
	}
	template<>
	void Scene::componentAdded(Entity& entity, TransformComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, WorldTransformComponent& comp) {
		// 비워둔다 — 값은 updateTransforms()가 채운다. 여기서 미리 채우면
		// 부모가 아직 안 붙은 시점의 값이라 어차피 다음 패스에 덮인다.
	}
	template<>
	void Scene::componentAdded(Entity& entity, RelationshipComponent& comp) {
		// 비워둔다 — 링크는 setParent()만 건드린다.
	}
	template<>
	void Scene::componentAdded(Entity& entity, CameraMatrixComponent& comp) {
		comp.sceneCam.set_proj_mat(pm_viewport_w, pm_viewport_h);
		//comp.sceneCam.cam_ctrl.bind();
	}
	template<>
	void Scene::componentAdded(Entity& entity, VisComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, TagComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, ShadingComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, MeshComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, NativeScriptComponent& comp) {

	}


	Scene::Scene()
	{
		//// texture ponters define
		//pm_checker_tex_ptr = Texture2D::create("assets/textures/checkerBoard_1.png");
		//pm_icon_tex_ptr = Texture2D::create("assets/textures/WORKMANs_icon_69.png");
				// default cam entities.
	}

	Scene::~Scene()
	{
		// ⚠️ registry가 파괴되면 MeshComponent의 Ref<VertexArray>가 0이 되고
		//    ~VulkanVertexBuffer가 그 자리에서 vmaDestroyBuffer를 부른다.
		//    New Scene / Open Scene이 옛 씬을 통째로 버리는 경로가 여기다.
		//    아래 deletEntity와 같은 이유로 GPU를 먼저 비운다.
		RendererCommand::waitIdle();
	}

	bool Scene::isValid(const Entity& e) const
	{
		const entt::entity h = e.get_entityID();
		return h != entt::null && pm_register.valid(h);
	}

	bool Scene::isSpatial(entt::entity e) const
	{
		return pm_register.all_of<TransformComponent, WorldTransformComponent>(e);
	}

	Entity Scene::createEntity(const std::string& name){
		Entity E = { pm_register.create(), this };
		// 이름만 붙인다. 마야도 계산 노드에 이름이 있고(skinCluster1),
		// 아웃라이너·직렬화가 이름은 무조건 가정한다.
		auto& tagC = E.addComponent<TagComponent>(name);
		tagC.name = name.empty() ? "Entity" : name;
		return E;
	}

	Entity Scene::createObject(const std::string& name)
	{
		Entity E = createEntity(name.empty() ? "Object" : name);
		E.addComponent<TransformComponent>();
		// 월드 캐시는 로컬과 한 세트다. 부모가 없으면 로컬의 사본일 뿐이지만,
		// 오브젝트 전부가 갖고 있어야 렌더 순회가 "계층 있음/없음" 분기 없이
		// view 하나로 돈다. RelationshipComponent는 반대로 붙을 때만 생긴다.
		E.addComponent<WorldTransformComponent>();
		E.addComponent<VisComponent>(1);
		return E;
	}

	void Scene::deletEntity( Entity& entity)
	{
		const entt::entity root = entity.get_entityID();
		if (root == entt::null || !pm_register.valid(root))
			return;

		// ⚠️ 지오메트리는 GPU 버퍼다. 컴포넌트가 파괴되는 순간 ~VulkanVertexBuffer가
		//    vmaDestroyBuffer를 바로 부르는데, 직전 프레임들이 아직 GPU에서 돌고 있고
		//    그 커맨드버퍼가 이 버퍼를 참조 중이다 (frames in flight = 2).
		//    → VUID-vkDestroyBuffer-buffer-00922
		//
		//    결정 ③(리사이즈 안전성)과 같은 처리다. 삭제는 사용자 조작이라 빈도가
		//    낮아 한 프레임 스톨을 감당할 수 있다.
		//
		// ⚠️ 이 방식의 수명은 여기까지다 — **매 프레임 파괴**가 생기면 못 버틴다.
		//    모디파이어가 슬라이더를 끄는 동안 지오메트리를 재생성하기 시작하면
		//    프레임마다 GPU를 세우게 된다. 그때 지연 삭제 큐(결정 ③의 B안)가
		//    선택이 아니라 필수가 된다.
		RendererCommand::waitIdle();

		// 부모의 children에서 먼저 뗀다. 안 떼면 부모가 죽은 핸들을 계속 들고 있다가
		// 다음 프레임 updateTransforms()가 그걸 그대로 따라간다 — 크래시다.
		detachFromParent(root);
		destroySubtree(root);
	}

	// 마야 방식(결정됨): 부모를 지우면 자손도 함께 사라진다.
	// 블렌더처럼 자식을 남기는 방식도 있지만, 그룹을 지웠는데 내용물이 화면에
	// 흩어져 남는 쪽이 모델링 작업에선 더 헷갈린다.
	void Scene::destroySubtree(entt::entity e)
	{
		// ⚠️ 복사본으로 순회한다. 재귀가 registry를 건드리는 순간 원본 vector가
		//    재할당되거나 통째로 소멸해 순회 중인 반복자가 무효화된다.
		std::vector<entt::entity> children;
		if (const auto* rel = pm_register.try_get<RelationshipComponent>(e))
			children = rel->children;

		for (auto child : children)
			destroySubtree(child);

		// 기본 카메라만은 예외다 — 지우면 툴바 콤보박스가 인덱싱하는
		// pm_cam_entities / pm_cameraViewArray 짝이 깨진다. 그래서 부모가 사라져도
		// 살려서 루트로 올린다. (sceneProperty.cpp의 "기본 카메라는 지울 수 없다"와 같은 판단)
		if (const auto* cam = pm_register.try_get<CameraMatrixComponent>(e);
			cam && cam->defaultCam) {
			// 부모가 곧 사라지므로 지금 보이는 위치를 로컬로 구워 넣는다.
			// 안 그러면 부모 변환을 잃고 화면에서 튄다.
			const glm::mat4 world = pm_register.get<WorldTransformComponent>(e).matrix;
			detachFromParent(e);
			pm_register.get<TransformComponent>(e).setLocalMatrix(world);
			APP_WMS_CORE_WARN("기본 카메라는 삭제하지 않는다 — 루트로 옮긴다.");
			return;
		}

		removeFromCameraLists(e);
		pm_register.destroy(e);
	}

	void Scene::removeFromCameraLists(entt::entity e)
	{
		// pm_cam_entities와 pm_cameraViewArray는 인덱스가 짝을 이룬다
		// (mainWindow의 콤보박스가 selectedIndex 하나로 둘 다 인덱싱한다).
		// 예전엔 한쪽은 이름으로, 다른 쪽은 id로 지워서 동명 엔티티가 있으면
		// 두 배열이 조용히 어긋났다. 같은 인덱스로 함께 지운다.
		auto it = std::find_if(pm_cam_entities.begin(), pm_cam_entities.end(),
			[e](const Entity& cam) { return cam.get_entityID() == e; });
		if (it == pm_cam_entities.end())
			return;

		const size_t index = (size_t)std::distance(pm_cam_entities.begin(), it);
		pm_cam_entities.erase(it);
		if (index < pm_cameraViewArray.size())
			pm_cameraViewArray.erase(pm_cameraViewArray.begin() + index);
	}


	// ─────────────────────────────────────────────────────────────
	//  계층 — 월드 행렬 갱신
	// ─────────────────────────────────────────────────────────────

	void Scene::updateTransforms()
	{
		// 루트(부모 없음)만 골라 내려간다. 자식은 부모의 재귀가 처리하므로
		// 여기서 또 만나면 중복 계산이 된다.
		auto view = pm_register.view<TransformComponent, WorldTransformComponent>();
		for (auto entity : view) {
			const auto* rel = pm_register.try_get<RelationshipComponent>(entity);
			if (rel && rel->parent != entt::null)
				continue;
			updateWorldTransform(entity, nullptr, 0);
		}
	}

	void Scene::updateWorldTransform(entt::entity e, const glm::mat4* parentWorld, uint32_t parentVersion)
	{
		// ⚠️ 자식 목록은 순수 토폴로지라 위치 없는 엔티티(모디파이어 등)도 들어올 수 있다.
		//    그런 엔티티에 get<TransformComponent>를 걸면 그 자리에서 터진다.
		//    애초에 그런 걸 트랜스폼 계층에 매다는 건 잘못된 사용이지만,
		//    "잘못 쓰면 크래시"가 아니라 "잘못 쓰면 아무 일도 안 남"으로 만든다.
		if (!isSpatial(e))
			return;

		auto& local = pm_register.get<TransformComponent>(e);
		auto& world = pm_register.get<WorldTransformComponent>(e);

		// 순서 주의 — getVersion()이 내부에서 갱신까지 끝낸다.
		const uint32_t localVersion = local.getVersion();
		const glm::mat4& localMatrix = local.getLocalMatrix();

		// 로컬도 그대로고 부모 월드도 그대로면 결과가 같다 → 곱을 건너뛴다.
		// 세대를 안 올리므로 자식들도 연쇄적으로 같은 판정을 받아 건너뛴다.
		// (이게 "dirty 전파"의 실제 형태다 — 표시를 심는 게 아니라 세대를 비교한다)
		if (world.localVersionUsed != localVersion || world.parentVersionUsed != parentVersion) {
			// 루트는 parentWorld가 nullptr다. 단위행렬을 곱하지 않고 그냥 복사한다 —
			// 계층 없는 씬에서 엔티티마다 의미 없는 mat4 곱이 하나씩 붙는 걸 막는다.
			world.matrix = parentWorld ? (*parentWorld * localMatrix) : localMatrix;
			world.localVersionUsed = localVersion;
			world.parentVersionUsed = parentVersion;
			++world.version;
		}

		if (const auto* rel = pm_register.try_get<RelationshipComponent>(e)) {
			// 자식 목록은 재귀 중에 안 바뀐다(여기선 계층을 편집하지 않는다).
			// 그래서 복사 없이 그대로 순회해도 안전하다 — destroySubtree와 다른 점.
			for (auto child : rel->children)
				updateWorldTransform(child, &world.matrix, world.version);
		}
	}


	// ─────────────────────────────────────────────────────────────
	//  계층 — 토폴로지 편집
	// ─────────────────────────────────────────────────────────────

	bool Scene::setParent(Entity child, Entity parent, bool keepWorldTransform)
	{
		const entt::entity c = child.get_entityID();
		const entt::entity p = parent ? parent.get_entityID() : entt::null;

		if (c == entt::null || !pm_register.valid(c))
			return false;
		if (c == p) {
			APP_WMS_CORE_WARN("자기 자신을 부모로 삼을 수 없다.");
			return false;
		}
		// 순환 차단 — 새 부모가 이미 내 자손이면 A→B→A가 만들어지고,
		// updateWorldTransform의 재귀가 빠져나오지 못해 스택을 태운다.
		// 조용한 오동작이 아니라 즉사라서 여기서 반드시 막아야 한다.
		if (p != entt::null && isDescendantOf(p, c)) {
			APP_WMS_CORE_WARN("자손을 부모로 삼을 수 없다 (계층 순환).");
			return false;
		}

		// 위치가 없는 엔티티는 보정할 월드 자체가 없다.
		if (!isSpatial(c))
			keepWorldTransform = false;

		// 링크를 바꾸기 직전의 월드. 아래 보정의 기준값이다.
		// ⚠️ 위치 없는 엔티티엔 이 컴포넌트가 없으므로 보정할 때만 읽는다.
		glm::mat4 worldBefore(1.0f);
		if (keepWorldTransform) {
			// ⚠️ 기준이 되는 월드 값이 최신이어야 한다. 이 함수는 씬 구성 중이나
			//    onImGuiRender(=onUpdate의 패스가 끝난 뒤)에서 불리는데, 그 사이 패널이
			//    로컬을 만졌을 수 있다. 낡은 월드로 보정하면 물체가 엉뚱한 데로 튄다.
			//    여러 개를 한 번에 붙이는 경로가 생기면 이 호출을 루프 밖으로 빼야 한다(O(N·M)).
			updateTransforms();
			worldBefore = pm_register.get<WorldTransformComponent>(c).matrix;
		}

		detachFromParent(c);

		if (p != entt::null) {
			// ⚠️ 두 emplace를 먼저 끝내고 그 다음에 참조를 잡는다.
			//    c와 p는 같은 RelationshipComponent 풀에 들어가므로, 참조를 쥔 채
			//    다른 엔티티에 emplace하면 풀이 재할당되며 그 참조가 죽는다.
			//    ECS에서 흔한 함정이고 증상은 조용한 메모리 오염이다.
			//    (void) — 여기선 반환 참조를 일부러 버린다. 그걸 쥐는 게 바로 위험이다.
			(void)pm_register.get_or_emplace<RelationshipComponent>(c);
			(void)pm_register.get_or_emplace<RelationshipComponent>(p);
			pm_register.get<RelationshipComponent>(c).parent = p;
			pm_register.get<RelationshipComponent>(p).children.push_back(c);
		}

		if (keepWorldTransform) {
			// 마야 parent / 블렌더 Ctrl+P 관례 — 부모를 붙여도 물체는 화면에서 안 움직인다.
			// "붙기 전 월드를 그대로 유지하라"는 요구라서 기즈모와 완전히 같은 연산이다.
			// 링크는 위에서 이미 갈아끼웠으므로 setWorldMatrix가 **새** 부모를 보고 역산한다.
			setWorldMatrix(child, worldBefore);
		}
		return true;
	}

	void Scene::removeParent(Entity child, bool keepWorldTransform)
	{
		// 빈 Entity → operator bool()이 false → 부모를 entt::null로 잡는다.
		setParent(child, Entity{}, keepWorldTransform);
	}

	Entity Scene::getParent(Entity child)
	{
		const auto* rel = pm_register.try_get<RelationshipComponent>(child.get_entityID());
		if (!rel || rel->parent == entt::null)
			return Entity{};
		return Entity{ rel->parent, this };
	}

	const std::vector<entt::entity>* Scene::getChildren(Entity e) const
	{
		const auto* rel = pm_register.try_get<RelationshipComponent>(e.get_entityID());
		return rel ? &rel->children : nullptr;
	}

	const glm::mat4& Scene::getWorldMatrix(Entity e)
	{
		return pm_register.get<WorldTransformComponent>(e.get_entityID()).matrix;
	}

	void Scene::setWorldMatrix(Entity e, const glm::mat4& world)
	{
		const entt::entity h = e.get_entityID();
		if (h == entt::null || !pm_register.valid(h))
			return;
		// 위치가 없는 엔티티엔 지정할 월드가 없다 (모디파이어 등).
		if (!isSpatial(h))
			return;

		const auto* rel = pm_register.try_get<RelationshipComponent>(h);
		if (!rel || rel->parent == entt::null) {
			// 루트면 월드가 곧 로컬이다. 단위행렬 역행렬을 곱하는 낭비를 피한다 —
			// 기즈모를 끄는 동안 매 프레임 도는 경로다.
			pm_register.get<TransformComponent>(h).setLocalMatrix(world);
			return;
		}

		const glm::mat4& parentWorld = pm_register.get<WorldTransformComponent>(rel->parent).matrix;
		pm_register.get<TransformComponent>(h).setLocalMatrix(glm::inverse(parentWorld) * world);
	}

	void Scene::detachFromParent(entt::entity e)
	{
		auto* rel = pm_register.try_get<RelationshipComponent>(e);
		if (!rel || rel->parent == entt::null)
			return;

		// try_get은 조회만 한다(emplace 없음) → 풀 재할당이 없어 rel이 살아 있다.
		if (auto* parentRel = pm_register.try_get<RelationshipComponent>(rel->parent)) {
			auto& siblings = parentRel->children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), e), siblings.end());
		}
		rel->parent = entt::null;
	}

	bool Scene::isDescendantOf(entt::entity maybeDescendant, entt::entity ancestor) const
	{
		// 부모 체인을 위로 훑는다. 계층 깊이만큼만 돈다.
		entt::entity cursor = maybeDescendant;
		while (cursor != entt::null) {
			if (cursor == ancestor)
				return true;
			const auto* rel = pm_register.try_get<RelationshipComponent>(cursor);
			if (!rel)
				return false;
			cursor = rel->parent;
		}
		return false;
	}


	void Scene::onUpdate(Timer ts) {

		//// cameraController entity  <----- pm_register <--- viewport
		//pm_register.view<NativeScriptComponent>().each([=](auto entity, auto& nativeScriptComp)
		//{
		//	if (!nativeScriptComp.scriptable_entt_ptr)
		//	{
		//		nativeScriptComp.instantiate_func();
		//		nativeScriptComp.scriptable_entt_ptr->pm_entity = Entity{ entity, this };
		//		nativeScriptComp.onCreate_func(nativeScriptComp.scriptable_entt_ptr);
		//	}
		//	if (!blocked) {
		//		nativeScriptComp.onUpdate_func(nativeScriptComp.scriptable_entt_ptr, ts);
		//	}
		//});

		// ── 1단계. 입력 → 로컬 쓰기 ─────────────────────────────────────
		//
		// ⚠️ 순서가 이 함수의 전부다. 카메라 컨트롤러는 이 안에서 트랜스폼을
		//    **쓰는** 유일한 주체다(translate/rotate 직접 대입). 월드 갱신보다
		//    먼저 돌려야 이번 프레임의 조작이 이번 프레임 화면에 반영된다.
		//    뒤로 밀면 카메라가 한 프레임 늦게 따라와 마우스가 미끄러지는 느낌이 난다.
		//
		// TagComponent는 안 쓰므로 view에서 뺐다. createEntity가 모두에게 붙이므로
		// 매칭되는 엔티티 집합은 그대로다.
		auto camGroup = pm_register.view<TransformComponent, CameraMatrixComponent>();
		CameraController* main_cam = nullptr;
		entt::entity primaryCam = entt::null;
		for (auto& entity : camGroup) {
			auto [transform_fromComp, cameraMat_fromComp] =
				camGroup.get<TransformComponent, CameraMatrixComponent>(entity);
			if (cameraMat_fromComp.primary == true) {
				main_cam = &cameraMat_fromComp.sceneCam;
				if (!blocked) {
					main_cam->onUpdate(ts, transform_fromComp.translate, transform_fromComp.rotate);
					// ⚠️ 컨트롤러는 yaw/pitch를 오일러 필드에 직접 쓴다. 이 카메라가
					//    쿼터니언 모드면 getLocalMatrix()가 그 필드를 안 읽으므로
					//    조작이 통째로 무시된다(화면은 멀쩡한데 tumble이 안 먹는다).
					//    컨트롤러 값은 본질적으로 XYZ 오일러라 그대로 변환해 넘기면
					//    의도가 보존된다.
					if (transform_fromComp.getRotationMode() == RotationMode::Quaternion)
						transform_fromComp.rotateQuat =
							eulerToQuat(glm::radians(transform_fromComp.rotate), RotationMode::EulerXYZ);
				}
				primaryCam = entity;
				break;
			}
		}

		// ── 2단계. 로컬 → 월드 ──────────────────────────────────────────
		// 방금 쓴 카메라 로컬까지 여기서 접힌다.
		updateTransforms();

		// ── 3단계. 월드 읽기 → 렌더 ─────────────────────────────────────
		if (main_cam && primaryCam != entt::null) {
			// ⚠️ 렌더는 _vk(벌칸 NDC, Y 아래) 쪽을 쓴다. ImGuizmo는 원본을 쓴다.
			//    둘은 일부러 다르다 — cameraController.h의 두 게터 주석 참고.
			//
			// 뷰 = 카메라의 **월드** 역행렬. 로컬을 쓰면 카메라를 리그에 붙였을 때
			// 부모 변환이 통째로 빠져 화면이 엉뚱한 데를 본다.
			const glm::mat4& camWorld = pm_register.get<WorldTransformComponent>(primaryCam).matrix;
			Renderer::beginScene(main_cam->get_projection_vk() * glm::inverse(camWorld));

			// 폴리곤 메쉬 경로. 커브·NURBS는 렌더 방식이 달라 별도 순회로 붙는다.
			//
			// 🟢 TransformComponent가 view에서 빠졌다 — 여기선 이미 접힌 결과만 있으면 된다.
			//    이게 월드를 별도 컴포넌트로 분리한 이유다. 로컬 36B와 로컬캐시 100B를
			//    매 엔티티마다 메모리 버스로 끌고 오지 않는다.
			auto meshes = pm_register.view<WorldTransformComponent, MeshComponent>();
			for (auto& entity : meshes) {
				auto [worldComp, meshComp] =
					meshes.get<WorldTransformComponent, MeshComponent>(entity);
				if (!meshComp.mesh) continue;
				// 🟢 엔티티 핸들 원본(index+version)을 그대로 보낸다. 버전 비트까지
				//    보존되므로 P-b에서 읽은 값을 entt::entity로 되돌리면 정확히 이 엔티티다.
				Renderer::submit(meshComp.mesh, worldComp.matrix,
					(int32_t)(uint32_t)entity);
			}

			Renderer::end();
		}
	}

	void Scene::event(Event& e)
	{
	}



	void Scene::onAttach()
	{
		add_camMat_entt("perspective", SceneCamera::CameraProjType::perspective, true);
		add_camMat_entt("top", SceneCamera::CameraProjType::orthographic, true);
		add_camMat_entt("front", SceneCamera::CameraProjType::orthographic, true);
		add_camMat_entt("side", SceneCamera::CameraProjType::orthographic, true);

		// 🚧 [H-b 검증용 부모-자식 큐브] 카메라가 원점에 있으므로 앞쪽(-Z)에 놓는다.
		//    메뉴(Create > Polygon > Cube)로도 추가할 수 있게 되면 지워도 된다.
		//
		// Cube::create()는 "큐브라는 종류"가 아니라 초기 형상을 찍어내는 팩토리다.
		// 결과물은 그냥 각 엔티티가 소유하는 폴리곤 메쉬다.
		Entity parentCube = createObject("Cube_Parent");
		parentCube.addComponent<MeshComponent>(Cube::create());
		auto& parentTransform = parentCube.getComponent<TransformComponent>();
		parentTransform.translate = { -1.2f, 0.0f, -6.0f };
		// 정면으로 두면 한 면만 보여서 원근·깊이·조명이 맞는지 판별이 안 된다.
		// 비스듬히 돌려 세 면이 동시에 보이게 한다.
		parentTransform.rotate = { 25.0f, 35.0f, 0.0f };

		Entity childCube = createObject("Cube_Child");
		childCube.addComponent<MeshComponent>(Cube::create());
		auto& childTransform = childCube.getComponent<TransformComponent>();
		// 이 값은 **부모 기준**이다. 부모의 회전까지 타고 도므로 월드에선
		// 오른쪽이 아니라 부모가 기울어진 방향으로 떨어져 붙는다 — 그게 계층이 걸렸다는 증거다.
		childTransform.translate = { 2.2f, 0.0f, 0.0f };
		childTransform.scale = { 0.5f, 0.5f, 0.5f };

		// ⚠️ keepWorldTransform = false.
		//    보정(true)은 **이미 월드에 놓인 물체**를 화면에서 안 움직이게 붙일 때 쓴다.
		//    여기선 방금 로컬을 "부모 기준"으로 직접 넣었으므로 보정하면 그 의도가 지워진다.
		setParent(childCube, parentCube, false);
	}



	void Scene::resizeView(uint32_t w, uint32_t h) {
		pm_viewport_w = w;
		pm_viewport_h = h;
		auto camera_entts = pm_register.view<CameraMatrixComponent>();
		for (auto& camera_entity : camera_entts) {
			auto& camComp = camera_entts.get<CameraMatrixComponent>(camera_entity);
			if(!camComp.fixed_aspectRatio)
				camComp.sceneCam.set_proj_mat(w, h);
		}

	}


	void Scene::add_camMat_entt(const std::string& entt_tagName, SceneCamera::CameraProjType type,
		bool defaultCam)
	{
		// 카메라도 공간에 위치가 있다 — 뷰 행렬이 그 월드의 역행렬이다.
		Entity camMat_entt = createObject(entt_tagName);
		auto& camMat_comp = camMat_entt.addComponent<CameraMatrixComponent>();
		//camMat_entt.addComponent<NativeScriptComponent>().bind<CameraController>();
		
		pm_cam_entities.emplace_back(camMat_entt);
		// set cam projtion type!!!
		camMat_comp.sceneCam.set_camProjType(type);
		pm_cameraViewArray.emplace_back(entt_tagName);

		if (type == SceneCamera::CameraProjType::perspective) {
			camMat_comp.primary = true;
			//camMat_entt.getComponent<TransformComponent>().translate = { 0.0f,0.0f,0.0f };
		}

		if (defaultCam == false) {
			camMat_comp.primary = true;
		}
		else {
			auto& tagComp = camMat_entt.getComponent<TagComponent>();
			tagComp.nameEditable = false;
			auto& visComp = camMat_entt.getComponent<VisComponent>();
			visComp.visible = false;
		}
		camMat_comp.defaultCam = defaultCam;
	}

	Entity* Scene::get_camera_primary()
	{
		for (auto& cam_entt : pm_cam_entities) {
			if (cam_entt.getComponent<CameraMatrixComponent>().primary) {
				return &cam_entt;
			}
		}

		return nullptr;
	}




}


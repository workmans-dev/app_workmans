#include "pch.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include <glm/glm.hpp>
// The scene touches nearly every component type (traversal + componentAdded specializations) -> the umbrella is honest here
#include "scene/components/components.h"
#include "renderer/geometry/cube.h"


namespace AppWMs {

	// ─────────────────────────────────────────────────────────────
	//  componentAdded hook -- where the scene steps in the moment a component is attached to an entity
	//
	//  WARNING: these specializations must appear above the first use (onAttach -> addComponent).
	//     An explicit specialization that comes after an implicit instantiation is ill-formed.
	//     They used to sit at the bottom of the file and happened to pass because addComponent
	//     returned T&, so MSVC deferred instantiating the body. Switching to decltype(auto)
	//     exposed it at once (the body must be instantiated immediately to know the return type).
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
		// Left empty -- updateTransforms() fills the value. Filling it here would be a value from
		// before the parent was attached, and the next pass overwrites it anyway.
	}
	template<>
	void Scene::componentAdded(Entity& entity, RelationshipComponent& comp) {
		// Left empty -- only setParent() touches the links.
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
		// WARNING: when the registry is destroyed, MeshComponent's Ref<VertexArray> drops to 0 and
		//    ~VulkanVertexBuffer calls vmaDestroyBuffer right there.
		//    This is the path New Scene / Open Scene take when they throw the old scene away.
		//    The GPU is drained first, for the same reason as deletEntity below.
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
		// Only a name is attached. Maya names its computation nodes too (skinCluster1), and
		// the outliner and serialization assume a name unconditionally.
		auto& tagC = E.addComponent<TagComponent>(name);
		tagC.name = name.empty() ? "Entity" : name;
		return E;
	}

	Entity Scene::createObject(const std::string& name)
	{
		Entity E = createEntity(name.empty() ? "Object" : name);
		E.addComponent<TransformComponent>();
		// The world cache is a set with the local. With no parent it is merely a copy of the local,
		// but every object has to have it for the render traversal to run on one view with no
		// "has hierarchy" branch. RelationshipComponent, by contrast, appears only when attached.
		E.addComponent<WorldTransformComponent>();
		E.addComponent<VisComponent>(1);
		return E;
	}

	void Scene::deletEntity( Entity& entity)
	{
		const entt::entity root = entity.get_entityID();
		if (root == entt::null || !pm_register.valid(root))
			return;

		// WARNING: geometry is a GPU buffer. The moment the component is destroyed,
		//    ~VulkanVertexBuffer calls vmaDestroyBuffer, while the previous frames are still running
		//    on the GPU with command buffers that reference this buffer (frames in flight = 2).
		//    → VUID-vkDestroyBuffer-buffer-00922
		//
		//    Same treatment as decision #3 (resize safety). Deletion is a user action, so it is
		//    infrequent enough to absorb a one-frame stall.
		//
		// WARNING: this approach's lifetime ends here -- it cannot survive *per-frame destruction*.
		//    Once a modifier starts regenerating geometry while a slider is dragged, the GPU gets
		//    stalled every frame. At that point a deferred destruction queue (option B of
		//    decision #3) stops being a choice and becomes mandatory.
		RendererCommand::waitIdle();

		// Detach from the parent's children first. Without that the parent keeps holding a dead
		// handle and the next frame's updateTransforms() follows it straight in -- a crash.
		detachFromParent(root);
		destroySubtree(root);
	}

	// The Maya way (decided): deleting a parent takes the descendants with it.
	// Blender's way of leaving the children behind is an option, but having the contents scatter
	// across the screen after deleting a group is more confusing in modeling work.
	void Scene::destroySubtree(entt::entity e)
	{
		// WARNING: iterate over a copy. The moment the recursion touches the registry, the original
		//    vector may be reallocated or destroyed outright, invalidating the iterator in use.
		std::vector<entt::entity> children;
		if (const auto* rel = pm_register.try_get<RelationshipComponent>(e))
			children = rel->children;

		for (auto child : children)
			destroySubtree(child);

		// The default cameras alone are the exception -- deleting one breaks the
		// pm_cam_entities / pm_cameraViewArray pairing that the toolbar combo box indexes. So even
		// when the parent goes they survive, lifted to the root. (Same judgement as "default cameras cannot be deleted" in sceneProperty.cpp)
		if (const auto* cam = pm_register.try_get<CameraMatrixComponent>(e);
			cam && cam->defaultCam) {
			// The parent is about to disappear, so bake the currently visible position into the local.
			// Otherwise it loses the parent transform and jumps on screen.
			const glm::mat4 world = pm_register.get<WorldTransformComponent>(e).matrix;
			detachFromParent(e);
			pm_register.get<TransformComponent>(e).setLocalMatrix(world);
			APP_WMS_CORE_WARN("Default cameras are not deleted - moving it to the root.");
			return;
		}

		removeFromCameraLists(e);
		pm_register.destroy(e);
	}

	void Scene::removeFromCameraLists(entt::entity e)
	{
		// pm_cam_entities and pm_cameraViewArray are paired by index
		// (mainWindow's combo box indexes both with a single selectedIndex).
		// They used to be erased by name on one side and by id on the other, so with same-named
		// entities the two arrays silently diverged. They are erased together, by the same index.
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
	//  Hierarchy -- world matrix update
	// ─────────────────────────────────────────────────────────────

	void Scene::updateTransforms()
	{
		// Only the roots (no parent) are picked and descended from. Children are handled by the
		// parent's recursion, so meeting them again here would be duplicate work.
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
		// WARNING: the child list is pure topology, so position-less entities (modifiers etc.) can be in it.
		//    Calling get<TransformComponent> on such an entity blows up right there.
		//    Hanging something like that off the transform hierarchy is wrong usage to begin with,
		//    but this turns "misuse crashes" into "misuse does nothing".
		if (!isSpatial(e))
			return;

		auto& local = pm_register.get<TransformComponent>(e);
		auto& world = pm_register.get<WorldTransformComponent>(e);

		// Mind the order -- getVersion() finishes the update internally.
		const uint32_t localVersion = local.getVersion();
		const glm::mat4& localMatrix = local.getLocalMatrix();

		// If the local is unchanged and the parent world is unchanged, the result is identical -> skip the multiply.
		// The generation is not bumped, so the children get the same verdict in turn and skip too.
		// (This is what "dirty propagation" actually looks like -- generations get compared, not marks planted.)
		if (world.localVersionUsed != localVersion || world.parentVersionUsed != parentVersion) {
			// A root has parentWorld == nullptr. It is copied rather than multiplied by identity --
			// that keeps a meaningless mat4 multiply off every entity in a hierarchy-free scene.
			world.matrix = parentWorld ? (*parentWorld * localMatrix) : localMatrix;
			world.localVersionUsed = localVersion;
			world.parentVersionUsed = parentVersion;
			++world.version;
		}

		if (const auto* rel = pm_register.try_get<RelationshipComponent>(e)) {
			// The child list does not change during the recursion (the hierarchy is not edited here),
			// so iterating it directly without a copy is safe -- the difference from destroySubtree.
			for (auto child : rel->children)
				updateWorldTransform(child, &world.matrix, world.version);
		}
	}


	// ─────────────────────────────────────────────────────────────
	//  Hierarchy -- topology editing
	// ─────────────────────────────────────────────────────────────

	bool Scene::setParent(Entity child, Entity parent, bool keepWorldTransform)
	{
		const entt::entity c = child.get_entityID();
		const entt::entity p = parent ? parent.get_entityID() : entt::null;

		if (c == entt::null || !pm_register.valid(c))
			return false;
		if (c == p) {
			APP_WMS_CORE_WARN("An entity cannot be its own parent.");
			return false;
		}
		// Cycle guard -- if the new parent is already my descendant, A->B->A is formed and the
		// recursion in updateWorldTransform cannot get out, burning the stack.
		// That is instant death rather than a silent misbehavior, so it has to be blocked here.
		if (p != entt::null && isDescendantOf(p, c)) {
			APP_WMS_CORE_WARN("A descendant cannot be the parent (hierarchy cycle).");
			return false;
		}

		// An entity with no position has no world to correct in the first place.
		if (!isSpatial(c))
			keepWorldTransform = false;

		// The world right before the links change. It is the reference value for the correction below.
		// WARNING: a position-less entity has no such component, so it is read only when correcting.
		glm::mat4 worldBefore(1.0f);
		if (keepWorldTransform) {
			// WARNING: the reference world value has to be current. This function is called during scene
			//    construction or from onImGuiRender (after onUpdate's pass has finished), and the panel
			//    may have touched the local in between. Correcting with a stale world sends the object somewhere wrong.
			//    If a path that attaches many at once appears, this call has to move out of the loop (O(N*M)).
			updateTransforms();
			worldBefore = pm_register.get<WorldTransformComponent>(c).matrix;
		}

		detachFromParent(c);

		if (p != entt::null) {
			// WARNING: finish both emplaces first, and only then take the references.
			//    c and p go into the same RelationshipComponent pool, so emplacing on another entity
			//    while holding a reference reallocates the pool and kills that reference.
			//    A common ECS trap, and the symptom is silent memory corruption.
			//    (void) -- the returned reference is deliberately discarded here. Holding it is the danger.
			(void)pm_register.get_or_emplace<RelationshipComponent>(c);
			(void)pm_register.get_or_emplace<RelationshipComponent>(p);
			pm_register.get<RelationshipComponent>(c).parent = p;
			pm_register.get<RelationshipComponent>(p).children.push_back(c);
		}

		if (keepWorldTransform) {
			// Maya parent / Blender Ctrl+P convention -- attaching a parent does not move the object on screen.
			// The requirement is "keep the pre-attach world", which makes it the same operation as the gizmo.
			// The links were already swapped above, so setWorldMatrix back-solves against the *new* parent.
			setWorldMatrix(child, worldBefore);
		}
		return true;
	}

	void Scene::removeParent(Entity child, bool keepWorldTransform)
	{
		// An empty Entity -> operator bool() is false -> the parent is taken as entt::null.
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
		// An entity with no position has no world to set (modifiers etc.).
		if (!isSpatial(h))
			return;

		const auto* rel = pm_register.try_get<RelationshipComponent>(h);
		if (!rel || rel->parent == entt::null) {
			// For a root the world is the local. This avoids wasting a multiply by an inverse identity --
			// it is the path that runs every frame while the gizmo is being dragged.
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

		// try_get only queries (no emplace) -> no pool reallocation, so rel stays alive.
		if (auto* parentRel = pm_register.try_get<RelationshipComponent>(rel->parent)) {
			auto& siblings = parentRel->children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), e), siblings.end());
		}
		rel->parent = entt::null;
	}

	bool Scene::isDescendantOf(entt::entity maybeDescendant, entt::entity ancestor) const
	{
		// Walks up the parent chain. It only loops as far as the hierarchy depth.
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

		// -- Stage 1. input -> write the local -------------------------------
		//
		// WARNING: the order is the whole point of this function. The camera controller is the only
		//    thing in here that *writes* a transform (assigning translate/rotate directly). It has to
		//    run before the world update for this frame's manipulation to land in this frame's image.
		//    Pushed later, the camera trails by one frame and the mouse feels like it is slipping.
		//
		// TagComponent is unused, so it was dropped from the view. createEntity attaches it to
		// everything, so the matching entity set is unchanged.
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
					// WARNING: the controller writes yaw/pitch straight into the Euler fields. If this camera
					//    is in quaternion mode, getLocalMatrix() does not read those fields, so the
					//    manipulation is ignored wholesale (the image looks fine but tumble does nothing).
					//    The controller values are XYZ Euler by nature, so converting them straight across
					//    preserves the intent.
					if (transform_fromComp.getRotationMode() == RotationMode::Quaternion)
						transform_fromComp.rotateQuat =
							eulerToQuat(glm::radians(transform_fromComp.rotate), RotationMode::EulerXYZ);
				}
				primaryCam = entity;
				break;
			}
		}

		// -- Stage 2. local -> world -----------------------------------------
		// The camera local just written gets folded in here too.
		updateTransforms();

		// -- Stage 3. read the world -> render -------------------------------
		if (main_cam && primaryCam != entt::null) {
			// WARNING: rendering uses the _vk side (Vulkan NDC, Y down). ImGuizmo uses the original.
			//    The two differ on purpose -- see the comments on the two getters in cameraController.h.
			//
			// view = the inverse of the camera's *world*. Using the local would drop the parent
			// transform entirely once the camera is attached to a rig, pointing the image somewhere wrong.
			const glm::mat4& camWorld = pm_register.get<WorldTransformComponent>(primaryCam).matrix;
			Renderer::beginScene(main_cam->get_projection_vk() * glm::inverse(camWorld));

			// The polygon mesh path. Curves and NURBS render differently and get their own traversal.
			//
			// TransformComponent dropped out of the view -- all that is needed here is the folded result.
			//    This is why the world went into a separate component. The 36B local and the 100B local
			//    cache are not dragged across the memory bus for every entity.
			auto meshes = pm_register.view<WorldTransformComponent, MeshComponent>();
			for (auto& entity : meshes) {
				auto [worldComp, meshComp] =
					meshes.get<WorldTransformComponent, MeshComponent>(entity);
				if (!meshComp.mesh) continue;
				// The raw entity handle (index+version) is sent as is. The version bits are preserved too,
				//    so casting the value read in P-b back to entt::entity gives exactly this entity.
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

		// TODO: [parent-child cubes for verifying H-b] The camera sits at the origin, so they go in front (-Z).
		//    Once they can also be added from the menu (Create > Polygon > Cube), this can go.
		//
		// Cube::create() is not "a kind called cube" but a factory that stamps out an initial shape.
		// The result is just a polygon mesh owned by each entity.
		Entity parentCube = createObject("Cube_Parent");
		parentCube.addComponent<MeshComponent>(Cube::create());
		auto& parentTransform = parentCube.getComponent<TransformComponent>();
		parentTransform.translate = { -1.2f, 0.0f, -6.0f };
		// Head-on, only one face is visible and there is no way to tell whether perspective, depth
		// and lighting are right. Rotated at an angle, three faces show at once.
		parentTransform.rotate = { 25.0f, 35.0f, 0.0f };

		Entity childCube = createObject("Cube_Child");
		childCube.addComponent<MeshComponent>(Cube::create());
		auto& childTransform = childCube.getComponent<TransformComponent>();
		// This value is *relative to the parent*. It rides the parent's rotation too, so in world
		// space it lands not to the right but along whichever way the parent is tilted -- the proof the hierarchy is wired.
		childTransform.translate = { 2.2f, 0.0f, 0.0f };
		childTransform.scale = { 0.5f, 0.5f, 0.5f };

		// ⚠️ keepWorldTransform = false.
		//    The correction (true) is for attaching an object *already placed in the world* without moving it on screen.
		//    Here the local was just set directly "relative to the parent", so correcting would erase that intent.
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
		// A camera has a position in space too -- the view matrix is the inverse of that world.
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


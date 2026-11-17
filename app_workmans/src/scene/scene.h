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
		// -- The two ways to make an entity --------------------------------
		//
		// WARNING: "what kind is this" is not held as an enum on TagComponent or anything like it.
		//    In an ECS, *which components it has* is the answer to that question.
		//    Holding it as an enum puts an if (type == ...) branch in every traversal and loses
		//    the contiguous-iteration benefit -- same logic as splitting shapes into mesh/curve/nurbs.
		//
		// createEntity : pure allocation. It only gets a name.
		//                The things with no position -- modifiers (mirror/skin), deformers,
		//                the "computing" entities -- come here. With no transform,
		//                view<TransformComponent> filters them out by itself.
		//
		// createObject : a scene object. It has a position in space and shows up on screen.
		//                Transform + WorldTransform + Vis are added.
		//
		// Maya draws the same line with DAG nodes (positioned) vs DG nodes (computing), Blender
		// with Object vs Modifier. For both, the criterion is "does it have a position".
		Entity createEntity(const std::string& name = std::string());
		Entity createObject(const std::string& name = std::string());
		// WARNING: the Maya way -- it deletes the whole subtree. Delete a parent and the descendants go too.
		//    The only exception is the default cameras (perspective/top/front/side). See the implementation comment.
		void deletEntity( Entity& entity);
		const entt::registry& get_reg() { return pm_register; }
		// Is this handle still alive? Because of subtree deletion (the Maya way), an entity
		// I never deleted can disappear -- deleting a parent kills its descendants with it.
		// So anything holding a list of Entity (a selection list, say) has to filter through this.
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
		// -- Hierarchy -----------------------------------------------------
		//
		// Fills the world matrices once per frame, root down to descendants.
		// Called at the very top of onUpdate. After this call, WorldTransformComponent is
		// the truth for this frame.
		//
		// Why one top-down pass rather than lazy evaluation at query time:
		// lazy walks up the parent chain by the full depth on every query. As bones get deeper,
		// reading one entity costs depth recursions. Top-down is exactly O(N), and it descends
		// to children with the parent's world already in hand, so nothing is revisited.
		void updateTransforms();

		// Moves child under parent. If parent is an empty Entity, child is pulled out to the root.
		//
		// With keepWorldTransform=true the child's local is corrected so its on-screen position
		// stays put (the convention of Maya parent / Blender Ctrl+P). With false the local is left
		// alone and it jumps by the parent transform.
		//
		// On failure (itself, or a descendant given as the parent = a cycle) it returns false and
		// changes nothing.
		bool setParent(Entity child, Entity parent, bool keepWorldTransform = true);
		void removeParent(Entity child, bool keepWorldTransform = true);
		// Returns an empty Entity (operator bool == false) when there is no parent.
		Entity getParent(Entity child);
		// The child list. nullptr when the entity is not wired into the hierarchy.
		// WARNING: the returned pointer dies if setParent or a deletion touches the vector.
		//    If the hierarchy will change mid-traversal, take a copy and walk that.
		const std::vector<entt::entity>* getChildren(Entity e) const;
		// The value updateTransforms() filled in this frame. For the local, use
		// TransformComponent::getLocalMatrix() directly.
		const glm::mat4& getWorldMatrix(Entity e);

		// Sets the world matrix directly. It back-solves the local so that world comes out under the parent.
		//     world = parentWorld × local   →   local = inverse(parentWorld) × world
		//
		// The gizmo (which manipulates in world) and setParent (which corrects so nothing moves) share this formula.
		// Writing it in both places would diverge the moment only one gets fixed -- "the gizmo is right
		// but parenting jumps" -- so it lives here alone.
		void setWorldMatrix(Entity e, const glm::mat4& world);

	private:
		// entt::entity : enum class
		entt::registry pm_register;
		// WARNING: the scene does not hold geometry as a shared copy. Each entity's MeshComponent
		//    must own its own for two objects to be editable separately.
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
		// Hierarchy internals. Why these take entt::entity rather than Entity --
		// the recursion touches the registry, so carrying one thin handle around is safer.
		void updateWorldTransform(entt::entity e, const glm::mat4* parentWorld, uint32_t parentVersion);
		void destroySubtree(entt::entity e);
		void detachFromParent(entt::entity e);
		void removeFromCameraLists(entt::entity e);
		bool isDescendantOf(entt::entity maybeDescendant, entt::entity ancestor) const;
		// Is this an entity in space (= was it made with createObject)?
		// Running a transform operation on a position-less entity blows up, so this guards the front.
		bool isSpatial(entt::entity e) const;


	};

}
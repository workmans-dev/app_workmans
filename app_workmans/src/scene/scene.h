#pragma once
#include "core/timer.h"
#include "core/core.h"
#include "renderer/texture.h"
#include "entt.hpp"
#include "renderer/renderer.h"
#include "renderer/camera/sceneCamera.h"

namespace AppWMs {
	class Entity;
	class Scene {
	public:
		Scene();
		~Scene();
		Entity createEntity(const std::string& name = std::string());
		void deletEntity(const Entity& entity);
		const entt::registry& get_reg() { return pm_register; }
		void onUpdate(Timer ts);
		void onAttach();
		void resizeView(uint32_t w, uint32_t h);
		void set_block_onUpdate(bool& fact) { blocked = fact; }
		void add_camMat_entt(const std::string& entt_tagName, SceneCamera::CameraProjType type,
			bool defaultCam);
		std::vector<std::string>& get_cameraNames(){ return pm_cameraViewArray;}
		std::vector<Entity>& get_camEntities() { return pm_cam_entities; }

	private:
		// entt::entity : enum class
		entt::registry pm_register;
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

	};

}
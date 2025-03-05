#pragma once
#include "Scene.h"
#include "entt.hpp"


namespace AppWMs {
	class Entity {
	public:
		Entity(const entt::entity& entityHandle, Scene* scene);
		Entity(const Entity& other)=default;
		Entity()=default;

		template <typename T, typename... Args>
		T& addComponent(Args&&... args) {

			return pm_scene->pm_register.emplace<T>(pm_entityHandle,
				std::forward<Args>(args)...);
		}

		template<typename T>
		T& getComponent() {
			return pm_scene->pm_register.get<T>(pm_entityHandle);
		}

		template<typename T>
		bool hasComponent() {
			return pm_scene->pm_register.all_of<T>(pm_entityHandle);
		}

		template<typename T>
		void removeComponent() {

			return pm_scene->pm_register.remove<T>(pm_entityHandle);
		}
		
		operator bool() const { return pm_entityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)pm_entityHandle; }
		bool operator == (const Entity& other) const {
			return pm_entityHandle == other.pm_entityHandle &&
				pm_scene == other.pm_scene;}
		bool operator != (const Entity& other) const {
			return !(*this == other);
		}
	private:
		Scene* pm_scene=nullptr;
		entt::entity pm_entityHandle{entt::null};
		
	};
}
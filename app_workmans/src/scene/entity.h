#pragma once
#include "Scene.h"
#include "entt.hpp"
#include <type_traits>


namespace AppWMs {
	class Entity {
	public:
		Entity(const entt::entity& entityHandle, Scene* scene);
		Entity(const Entity& other);
		Entity()=default;
		// WARNING: a tag component with no data members (CubeComponent and the like) is not
		//    actually stored by entt (empty-type optimization -> 0 bytes per entity). With no
		//    object to store, emplace returns void rather than a reference.
		//    So the return type has to branch:
		//      tag     -> void  (there is no reference to hand back)
		//      regular -> T&
		//    Being if constexpr, only one branch is instantiated per T, and decltype(auto)
		//    picks up the matching return type.
		template <typename T, typename... Args>
		decltype(auto) addComponent(Args&&... args) {
			if constexpr (std::is_empty_v<T>) {
				pm_scene->pm_register.emplace<T>(pm_entityHandle, std::forward<Args>(args)...);
				// There is no object to hand to the hook, so a temporary one is made. It is a value
				// that never gets stored, so anything written here is lost -- a tag hook should stay empty.
				T tag{};
				pm_scene->componentAdded<T>(*this, tag);
			}
			else {
				T& comp = pm_scene->pm_register.emplace<T>(pm_entityHandle, std::forward<Args>(args)...);
				pm_scene->componentAdded<T>(*this, comp);
				return (comp);   // parentheses are required -- they make decltype(auto) resolve to T&
			}
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
		bool hasComponent() const {
			return pm_scene->pm_register.all_of<T>(pm_entityHandle);
		}


		template<typename T>
		void removeComponent() {

			return pm_scene->pm_register.remove<T>(pm_entityHandle);
		}
		
		const entt::entity& get_entityID() const { return pm_entityHandle; }

		operator bool() const { return pm_entityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)pm_entityHandle; }
		bool operator == (const Entity& other) const {
			return pm_entityHandle == other.pm_entityHandle &&
				pm_scene == other.pm_scene;}
		bool operator != (const Entity& other) const {
			return !(*this == other);}
		operator entt::entity() const { return pm_entityHandle; }
		bool operator()(const Entity& lhs, const Entity& rhs) const {
			return lhs.pm_entityHandle < rhs.pm_entityHandle;  // Example comparison logic
		}
		bool operator<(const Entity& other) const {
			return pm_entityHandle < other.pm_entityHandle;  // For example, compare based on 'id'
		}

	private:
		Scene* pm_scene=nullptr;
		entt::entity pm_entityHandle{entt::null};
		
	};


	struct EntityComparator {
		bool operator()(const Entity& lhs, const Entity& rhs) const {
			return lhs.get_entityID() < rhs.get_entityID();  // Or any other unique property for comparison
		}
	};
}
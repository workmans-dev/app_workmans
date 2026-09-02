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
		// ⚠️ 데이터 멤버가 없는 태그 컴포넌트(CubeComponent 등)는 entt가 실제로
		//    저장하지 않는다(empty-type 최적화 → 엔티티당 0바이트). 저장할 실체가
		//    없으니 emplace가 참조가 아니라 void를 반환한다.
		//    그래서 반환 타입을 갈라야 한다:
		//      태그    → void  (돌려줄 참조 자체가 존재하지 않음)
		//      일반    → T&
		//    if constexpr이라 T마다 한쪽 가지만 인스턴스화되고, decltype(auto)가
		//    그에 맞는 반환 타입을 잡는다.
		template <typename T, typename... Args>
		decltype(auto) addComponent(Args&&... args) {
			if constexpr (std::is_empty_v<T>) {
				pm_scene->pm_register.emplace<T>(pm_entityHandle, std::forward<Args>(args)...);
				// 훅에 넘길 실체가 없으므로 임시로 하나 만든다. 저장되지 않는 값이라
				// 여기에 무언가를 쓰면 사라진다 — 태그용 훅은 비워두는 게 맞다.
				T tag{};
				pm_scene->componentAdded<T>(*this, tag);
			}
			else {
				T& comp = pm_scene->pm_register.emplace<T>(pm_entityHandle, std::forward<Args>(args)...);
				pm_scene->componentAdded<T>(*this, comp);
				return (comp);   // 괄호 필수 — decltype(auto)가 T&로 잡히게 한다
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
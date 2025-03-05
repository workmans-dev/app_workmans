#pragma once

#include "scene/entity.h"
namespace AppWMs {

	class ScriptableEntity {
	public:
		template<typename T>
		T& getComponent() {
			return pm_entity.getComponent<T>();
		}

	private:
		Entity pm_entity;
		friend class Scene;


	};
}
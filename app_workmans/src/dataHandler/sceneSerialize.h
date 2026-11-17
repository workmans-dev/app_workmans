#pragma once
#include "scene/scene.h"
#include "scene/entity.h"
// Serialization has to know every component type -> the umbrella is honest here
#include "scene/components/components.h"


namespace AppWMs {
	class Serialize {
	public:
		Serialize(const Ref<Scene>& scene) :pm_scene(scene) {}

		void serialize(const std::string& file_path);
		void runtime_serialize(const std::string& file_path);

		bool deserialize(const std::string& file_path);
		bool runtime_deserialize(const std::string& file_path);
		//virtual void serialize() = 0;
		//virtual void deserialize() = 0;

	private:
		Ref<Scene> pm_scene;
	};

}
#pragma once
#include "scene/scene.h"
#include "scene/entity.h"
// 직렬화는 모든 컴포넌트 타입을 알아야 한다 → 우산이 정직하다
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
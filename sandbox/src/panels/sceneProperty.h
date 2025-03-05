#pragma once

#include "core/core.h"
#include "scene/scene.h"
#include "scene/entity.h"


namespace AppWMs {

	class SceneProperties {
	public:
		SceneProperties() = default;
		SceneProperties(const Ref<Scene>& scene);
		void onImGuiRender();
		void setScene(const Ref<Scene>& scene);
		void showPropertyComponents(Entity& entity);
		void drawModel(Entity& entity);

	private:
		Ref<Scene> pm_scene;
		Entity pm_selectedNode;
		bool editMode = false;
		char editBuffer[256] = "";
	};
}
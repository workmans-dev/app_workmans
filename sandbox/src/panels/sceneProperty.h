#pragma once

#include "core/core.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include <set>  // For tracking selected nodes


namespace AppWMs {

	class SceneProperties {
	public:
		SceneProperties() = default;
		SceneProperties(const Ref<Scene>& scene);
		void onImGuiRender();
		void setScene(const Ref<Scene>& scene);
		void showPropertyComponents(const Entity& entity);
		void drawModel(Entity& entity);
		void set_deleteEntity(const Entity& entity);
		void set_vis_color(Entity& entity);
		void set_vis_ctrl();
	private:
		Ref<Scene> pm_scene;
		std::set<Entity, EntityComparator> pm_selectedEntities;
		bool editMode = false;
		char editBuffer[256] = "";
		bool clickedOnEntity = false;
	};

}
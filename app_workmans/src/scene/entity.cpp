#include "pch.h"
#include "scene/entity.h"




namespace AppWMs {
	Entity::Entity(const entt::entity& entityHandle, Scene* scene):
		pm_entityHandle(entityHandle), pm_scene(scene)
	{
	}
	Entity::Entity(const Entity& other):pm_entityHandle(other.pm_entityHandle),
		pm_scene(other.pm_scene)
	{
	}

}
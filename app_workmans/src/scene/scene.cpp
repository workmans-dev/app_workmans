#include "pch.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include <glm/glm.hpp>
#include "scene/component.h"
#include "renderer/renderer2D.h"


namespace AppWMs {

	static void doMath(const glm::mat4& transform) {

	}
	static void onTransformConstruct(entt::registry& registry, entt::entity entity){


	}

	

	Scene::Scene()
	{
		// texture ponters define
		pm_checker_tex_ptr = Texture2D::create("assets/textures/checkerBoard_1.png");
		pm_icon_tex_ptr = Texture2D::create("assets/textures/WORKMANs_icon_69.png");
	}

	Scene::~Scene()
	{
	}

	Entity Scene::createEntity(const std::string& name){
		Entity E = { pm_register.create(), this };
		E.addComponent<TransformComponent>();
		auto& tagC = E.addComponent<TagComponent>(name);
		tagC.name = name.empty() ? "Entity" : name;
		

		return E;
	}


	void Scene::onUpdate(Timer ts) {

		// camera, // getting pm_camMat_entt from pm_register <--- viewport
		pm_register.view<NativeScriptComponent>().each([=](auto entity, auto& nativeScriptComp)
		{
			if (!nativeScriptComp.scriptable_entt_ptr)
			{
				nativeScriptComp.instantiate_func();
				nativeScriptComp.scriptable_entt_ptr->pm_entity = Entity{ entity, this };
				nativeScriptComp.onCreate_func(nativeScriptComp.scriptable_entt_ptr);
			}
			if (!blocked) {
				nativeScriptComp.onUpdate_func(nativeScriptComp.scriptable_entt_ptr, ts);
			}
		});

		
		auto group = pm_register.view<TransformComponent, CameraMatrixComponent>();
		CameraMatrix* main_camMat=nullptr;
		glm::mat4* main_cam_transform = nullptr;
		for (auto& entity : group) {
			auto [transform_fromComp, cameraMat_fromComp] = 
				group.get<TransformComponent, CameraMatrixComponent>(entity);
			if (cameraMat_fromComp.primary==true) {
				main_cam_transform = &transform_fromComp.transform;
				main_camMat = &cameraMat_fromComp.sceneCam;
				break;
			}
		}

		if (main_camMat != nullptr) {
			Renderer2D::beginScene(*main_camMat, *main_cam_transform);
			auto group = pm_register.group<TransformComponent>(entt::get<ShadingComponent>);
			for (auto& e_ID : group) {
				auto [transform, shading] = group.get<TransformComponent, ShadingComponent>(e_ID);
				Renderer2D::drawQuad(transform, shading.color);
				//Renderer2D::drawQuad_texture("texture", transform, pm_checker_tex_ptr,
				//	{ 1.0f, 1.0f, 1.0f, 1.0f }, 10.0f);
				//Renderer2D::drawQuad_texture("texture", transform_icon, pm_icon_tex_ptr);
			}
			Renderer2D::endScene();
		}

	}

	void Scene::resizeView(uint32_t w, uint32_t h) {
		auto camera_entts = pm_register.view<CameraMatrixComponent>();
		for (auto& camera_entity : camera_entts) {
			auto& camComp = camera_entts.get<CameraMatrixComponent>(camera_entity);
			if(!camComp.fixed_aspectRatio)
				camComp.sceneCam.set_proj_mat(w, h);
		}

	}
}


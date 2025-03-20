#include "pch.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include <glm/glm.hpp>
#include "scene/component.h"
#include "renderer/renderer2D.h"


namespace AppWMs {



	Scene::Scene()
	{
		//// texture ponters define
		//pm_checker_tex_ptr = Texture2D::create("assets/textures/checkerBoard_1.png");
		//pm_icon_tex_ptr = Texture2D::create("assets/textures/WORKMANs_icon_69.png");
				// default cam entities.
	}

	Scene::~Scene()
	{
	}

	Entity Scene::createEntity(const std::string& name){
		Entity E = { pm_register.create(), this };
		E.addComponent<TransformComponent>();
		E.addComponent<VisComponent>(1);
		auto& tagC = E.addComponent<TagComponent>(name);
		tagC.name = name.empty() ? "Entity" : name;
		return E;
	}

	void Scene::deletEntity( Entity& entity)
	{
		auto itr = std::find_if(pm_cameraViewArray.begin(), pm_cameraViewArray.end(),
			[&entity](const std::string& Name) {return Name == entity.getComponent<TagComponent>().name;
			});
		if(itr != pm_cameraViewArray.end())
			pm_cameraViewArray.erase(itr);

		// Remove the entity from pm_cam_entities
		auto it_ = std::find_if(pm_cam_entities.begin(), pm_cam_entities.end(),
			[&entity](const Entity& e) {  return e.get_entityID() == entity.get_entityID(); });

		if (it_ != pm_cam_entities.end()) {
			pm_cam_entities.erase(it_);
		}

		pm_register.destroy(entity.get_entityID());

	}

	void Scene::onUpdate(Timer ts) {

		//// cameraController entity  <----- pm_register <--- viewport
		//pm_register.view<NativeScriptComponent>().each([=](auto entity, auto& nativeScriptComp)
		//{
		//	if (!nativeScriptComp.scriptable_entt_ptr)
		//	{
		//		nativeScriptComp.instantiate_func();
		//		nativeScriptComp.scriptable_entt_ptr->pm_entity = Entity{ entity, this };
		//		nativeScriptComp.onCreate_func(nativeScriptComp.scriptable_entt_ptr);
		//	}
		//	if (!blocked) {
		//		nativeScriptComp.onUpdate_func(nativeScriptComp.scriptable_entt_ptr, ts);
		//	}
		//});

		// camera component
		auto group = pm_register.view<TransformComponent, CameraMatrixComponent, TagComponent>();
		CameraController* main_cam=nullptr;
		glm::mat4 cam_transform;
		for (auto& entity : group) {
			auto [transform_fromComp, cameraMat_fromComp, tagComp] = 
				group.get<TransformComponent, CameraMatrixComponent, TagComponent>(entity);
			if (cameraMat_fromComp.primary==true) {
				main_cam = &cameraMat_fromComp.sceneCam;
				if (!blocked) {
					main_cam->onUpdate(ts, transform_fromComp.translate, transform_fromComp.rotate);
				}
				cam_transform = transform_fromComp.getTransform();

				break;
			}
		}
		// rendering
		if (main_cam) {

			Renderer2D::beginScene(*main_cam, cam_transform);
			auto group = pm_register.view<TransformComponent, ShadingComponent>();
			for (auto e_ID : group) {
				auto [transform, shading] = group.get<TransformComponent, ShadingComponent>(e_ID);
				Renderer2D::drawQuad(transform.getTransform(), shading.color);

				//Renderer2D::drawQuad_texture("texture", transform, pm_checker_tex_ptr,
				//	{ 1.0f, 1.0f, 1.0f, 1.0f }, 10.0f);
				//Renderer2D::drawQuad_texture("texture", transform_icon, pm_icon_tex_ptr);
			}
			Renderer2D::endScene();
		}

	}

	void Scene::event(Event& e)
	{
	}



	void Scene::onAttach()
	{
		add_camMat_entt("perspective", SceneCamera::CameraProjType::perspective, true);
		add_camMat_entt("top", SceneCamera::CameraProjType::orthographic, true);
		add_camMat_entt("front", SceneCamera::CameraProjType::orthographic, true);
		add_camMat_entt("side", SceneCamera::CameraProjType::orthographic, true);
	}



	void Scene::resizeView(uint32_t w, uint32_t h) {
		pm_viewport_w = w;
		pm_viewport_h = h;
		auto camera_entts = pm_register.view<CameraMatrixComponent>();
		for (auto& camera_entity : camera_entts) {
			auto& camComp = camera_entts.get<CameraMatrixComponent>(camera_entity);
			if(!camComp.fixed_aspectRatio)
				camComp.sceneCam.set_proj_mat(w, h);
		}

	}


	void Scene::add_camMat_entt(const std::string& entt_tagName, SceneCamera::CameraProjType type,
		bool defaultCam)
	{
		Entity camMat_entt = createEntity(entt_tagName);
		auto& camMat_comp = camMat_entt.addComponent<CameraMatrixComponent>();
		//camMat_entt.addComponent<NativeScriptComponent>().bind<CameraController>();
		
		pm_cam_entities.emplace_back(camMat_entt);
		// set cam projtion type!!!
		camMat_comp.sceneCam.set_camProjType(type);
		pm_cameraViewArray.emplace_back(entt_tagName);

		if (type == SceneCamera::CameraProjType::perspective) {
			camMat_comp.primary = true;
			//camMat_entt.getComponent<TransformComponent>().translate = { 0.0f,0.0f,0.0f };
		}

		if (defaultCam == false) {
			camMat_comp.primary = true;
		}
		else {
			auto& tagComp = camMat_entt.getComponent<TagComponent>();
			tagComp.nameEditable = false;
			auto& visComp = camMat_entt.getComponent<VisComponent>();
			visComp.visible = false;
		}
		camMat_comp.defaultCam = defaultCam;
	}

	Entity* Scene::get_camera_primary()
	{
		for (auto& cam_entt : pm_cam_entities) {
			if (cam_entt.getComponent<CameraMatrixComponent>().primary) {
				return &cam_entt;
			}
		}

		return nullptr;
	}




	template<typename T>
	void Scene::componentAdded(Entity& entity, T& comp) {
		static_assert(false);
	}
	template<>
	void Scene::componentAdded(Entity& entity, TransformComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, CameraMatrixComponent& comp) {
		comp.sceneCam.set_proj_mat(pm_viewport_w, pm_viewport_h);
		//comp.sceneCam.cam_ctrl.bind();
	}
	template<>
	void Scene::componentAdded(Entity& entity, VisComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, TagComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, ShadingComponent& comp) {

	}
	template<>
	void Scene::componentAdded(Entity& entity, NativeScriptComponent& comp) {

	}
}


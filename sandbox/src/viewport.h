#pragma once
#include "core/timer.h"
#include "core/app_workmans.h"
#include "panels/sceneProperty.h"



namespace AppWMs {
	class Viewport :public Layer{
	public:
		Viewport();
		virtual ~Viewport()= default;
		virtual void onAttach() override;
		virtual void onUpdate(Timer time) override;

		void onEvent(Event& event) override;

		virtual void onImGuiRender() override;
		void add_camMat_entt(const std::string& entt_tagName, SceneCamera::CameraProjType type);
		//void onAttach() override {
		//	std::cout << "test" << std::endl;
		//}

	private:
		void createToolBar();
		bool pm_viewportEventBlocking = false;
		//CameraController pm_cameraController;
		Ref<FrameBuffer> pm_frameBuffer;

		glm::vec2 pm_viewportSize = { 1024.0f, 768.0f };

		Entity g_sq_entt;
		Entity r_sq_entt;
		//Entity pm_camMat_entt;
		//Entity pm_camMat_t_entt;
		Ref<Scene> pm_scene;
		SceneProperties pm_sceneProperties_panel;

		int selectedCamera_index = 0;

		std::vector<Entity> pm_cam_entities;
		int pm_cam_entity_index = 0;
		//std::array<const char*, 5> pm_typeArray = SceneCamera::getCameraTypes();
		std::vector<std::string> pm_cameraViewArray;
		int selectedIndex = 0;

	};

}
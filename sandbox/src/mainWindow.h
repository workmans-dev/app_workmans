#pragma once
#include "core/timer.h"
#include "core/app_workmans.h"
#include "panels/sceneProperty.h"

// Decision #4, C1 -- no abstract Pipeline layer, so sandbox owns the Vulkan type directly.
// Once pipelines grow to two or three (solid/wireframe) and the axis shows, abstract it then.
#include "vulkan/vk_pipeline.h"



namespace AppWMs {
	class MainWindow :public Layer{
	public:
		MainWindow();
		virtual ~MainWindow()= default;
		void createMainWindow();
		void createMainMenu();
		void setViewport();
		virtual void onAttach() override;
		virtual void onUpdate(Timer time) override;
		void onEvent(Event& event) override;
		virtual void onImGuiRender() override;

		void createToolBar();
		void set_cam_comboBox();
		void create_cam_button(const float& w, const float& h);
		void add_newCam(SceneCamera::CameraProjType type);
		bool fileHandler_shortcut(KeyPressedEvent& keyEvent);

	public:
		// file handler
		void openScene();
		void saveAsScene();
		void newScene();

	private:
		Ref<RenderTarget> pm_viewportTarget;
		Scope<VulkanPipeline> pm_cubePipeline;
		Ref<Scene> pm_scene;
		glm::vec2 pm_viewportSize = { 1280.0f, 720.0f };
		SceneProperties pm_sceneProperties_panel;

	private:
		int selectedIndex = 0;
		bool dockSpace_open = true;
		bool showCameraNamePopup = false;
		bool pm_viewportEventBlocking = false;
		SceneCamera::CameraProjType type_=SceneCamera::CameraProjType::perspective;
		int pm_manipulatorType = -1;

		// WARNING: with picking, the click and the result are a few frames apart (GPU readback). So
		//    the modifier state is read *at the moment of the press* and held here, then used when
		//    the result ripens. Reading it again when the result arrives would find Shift released.
		SelectMode pm_pendingPickMode = SelectMode::Replace;

	};

}
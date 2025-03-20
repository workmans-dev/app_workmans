#pragma once
#include "core/timer.h"
#include "core/app_workmans.h"
#include "panels/sceneProperty.h"



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
		Ref<FrameBuffer> pm_frameBuffer;
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

	};

}
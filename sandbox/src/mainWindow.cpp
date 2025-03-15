#include "pch.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/imgui.h"

#include "debugs/instrumentation.h"
#include "mainWindow.h"

#include "dataHandler/sceneSerialize.h"
#include "dataHandler/fileHandler/fileDialog.h"


namespace AppWMs {

	MainWindow::MainWindow() : Layer("MainWindow") {

	}

	void MainWindow::createMainWindow()
	{
		static bool opt_fullscreen = true;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockSpace_open, window_flags);
		ImGui::PopStyleVar();
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		createMainMenu();
		ImGui::End();
	}

	void MainWindow::createMainMenu()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::Separator();
				if (ImGui::MenuItem("New Scene", "Ctrl+N", false, dockSpace_open != NULL)) {
					newScene();	
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Save As", "Ctrl+Shift+S", false, dockSpace_open != NULL)) {
					saveAsScene();
				}
				if (ImGui::MenuItem("Open Scene", "Ctrl+O", false, dockSpace_open != NULL)) {
					openScene();
				}
				if (ImGui::MenuItem("Exit", "Ctrl+Q", false, dockSpace_open != NULL)) {
					dockSpace_open = false;
					App::getApp().close();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Create"))
			{
				ImGui::Separator();
				{
					if (ImGui::BeginMenu("B-Spline")) {

						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("NURBS")) {
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Polygon")) {
						if (ImGui::MenuItem("Cube", NULL, false, dockSpace_open != NULL)) {

						}
						if (ImGui::MenuItem("Plane", NULL, false, dockSpace_open != NULL)) {
							Entity entt = pm_scene->createEntity("Plane1");
							entt.addComponent<ShadingComponent>(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
						}
						ImGui::EndMenu();
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void MainWindow::setViewport()
	{
		// ----- view port -------
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 4.0f, 4.0f });
		ImGui::Begin("Viewport");
		// tool bar
		createToolBar();

		const bool& fact_hovered = ImGui::IsWindowHovered();
		const bool& fact_focus = ImGui::IsWindowFocused();
		// event factor
		pm_viewportEventBlocking = !fact_hovered;
		App::getApp().get_ImGuiLayer()->blockEvent(pm_viewportEventBlocking);
		pm_scene->set_block_onUpdate(pm_viewportEventBlocking);
		const ImVec2& resized_viewportSize = ImGui::GetContentRegionAvail();
		pm_viewportSize = { resized_viewportSize.x, resized_viewportSize.y };
		uint32_t textureID = pm_frameBuffer->get_colorAttach();
		ImGui::Image(textureID, ImVec2{ pm_viewportSize.x, pm_viewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

		ImGui::PopStyleVar();
		ImGui::End();
	}

	void MainWindow::onAttach() {

		FrameBufferSpec FB_spec = { (uint32_t)pm_viewportSize.x,(uint32_t)pm_viewportSize.y };
		pm_frameBuffer = FrameBuffer::create(FB_spec);

		pm_scene = std::make_shared<Scene>();
		pm_sceneProperties_panel.setScene(pm_scene);
		pm_scene->onAttach();

	}

	void MainWindow::onUpdate(AppWMs::Timer time)
	{
		APP_WMS_PROFILE_FUNC();
		const FrameBufferSpec& f_spec = pm_frameBuffer->get_spec();
		if (pm_viewportSize != glm::vec2(f_spec.w, f_spec.h)) 
		{
			SCOPE_PROFILE_TIMER("Resizing view");
			pm_frameBuffer->resize((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
			pm_scene->resizeView((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
		}

		{
			SCOPE_PROFILE_TIMER("Rendering on update");
			// renderer
			pm_frameBuffer->bind();
			RendererCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
			RendererCommand::clear();
			// take all components into pm_scene which it has been created by "onAttach"!!!
			pm_scene->onUpdate(time);
			pm_frameBuffer->unbind();
		}
	}

	void MainWindow::onEvent(AppWMs::Event& event)
	{
		EventDispatcher event_dispatcher(event);
		event_dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(MainWindow::fileHandler_shortcut));


	}

	void MainWindow::onImGuiRender()
	{
		// ----- main frame and window.
		createMainWindow();

		//----- propery editor! it will show components of the entity(property)
		pm_sceneProperties_panel.onImGuiRender();
		
		// ----- view port -------
		// it contains the createToolBar!
		setViewport();

	}

	void MainWindow::createToolBar()
	{
		// Define the toolbar and combo box sizes
		const float toolbarHeight = 15.0f;
		const float comboBoxWidth = 300.0f;
		// Create the toolbar container
		ImVec2 toolbarSize = ImVec2(ImGui::GetContentRegionAvail().x, toolbarHeight);
		ImGui::BeginChild("Toolbar", toolbarSize, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY,
			ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

		// -----------camera panel 
		create_cam_button(70.0f, toolbarHeight);
		ImGui::SameLine();
		// Create and set the combo box
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // Remove padding
		set_cam_comboBox();
		ImGui::PopStyleVar();
		ImGui::EndChild();
		// -----------camera panel end

	}

	void MainWindow::set_cam_comboBox()
	{
		std::vector<std::string>& cameraNames = pm_scene->get_cameraNames();
		std::vector<Entity>& camEntities = pm_scene->get_camEntities();
		// Ensure selectedIndex is valid, in case camera is removed or added
		if (selectedIndex >= cameraNames.size()) {
			selectedIndex = 0; // Reset to the first index if out of bounds
			camEntities[selectedIndex].getComponent<CameraMatrixComponent>().primary = true;
		}
		if (ImGui::BeginCombo("##CameraCombo", cameraNames[selectedIndex].c_str(),
			ImGuiComboFlags_HeightSmall)) {
			for (int i = 0; i < camEntities.size(); i++) {
				if (i == 1) {
					// default : front, side, top
					ImGui::TextDisabled("Orthographic"); // Non-selectable header
					ImGui::SameLine();
					float cursorY = ImGui::GetCursorPosY();
					ImGui::SetCursorPosY(cursorY + 6.0f); // Small vertical alignment
					ImGui::Separator();
				}
				else if (i >= 4) {
					ImGui::TextDisabled("User camera");
				}

				bool isSelected = (selectedIndex == i) ? true : false;
				if (ImGui::Selectable(cameraNames[i].c_str(), isSelected)) {
					selectedIndex = i;
					for (auto entity : camEntities) {
						entity.getComponent<CameraMatrixComponent>().primary = false;
					}
					// Now set the primary flag for the selected camera
					camEntities[selectedIndex].getComponent<CameraMatrixComponent>().primary = true;
				}
			}
			ImGui::EndCombo();
		}
	}

	void MainWindow::create_cam_button(const float& w, const float& h) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		if (ImGui::Button("Camera :", ImVec2(w, h))) {
			ImGui::OpenPopup("CreateCamera"); // Open the popup
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginPopup("CreateCamera")) {
			if (ImGui::MenuItem("Perspective")) {
				showCameraNamePopup = true;
				type_ = SceneCamera::CameraProjType::perspective;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Orthographic")) {
				showCameraNamePopup = true;
				type_ = SceneCamera::CameraProjType::orthographic;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		if (showCameraNamePopup) {
			ImGui::OpenPopup("CameraNamePopup");
			showCameraNamePopup = false;
		}

		// pop up wid, and create a camera
		add_newCam(type_);
	}


	void MainWindow::add_newCam(SceneCamera::CameraProjType type) {
		std::vector<std::string>& cameraNames = pm_scene->get_cameraNames();
		std::vector<Entity>& cameraEntities = pm_scene->get_camEntities();
		if (ImGui::BeginPopup("CameraNamePopup")) {
			static char inputText[128] = "";  // Input buffer for the camera name
			ImGui::Text("Enter Camera Name:");
			ImGui::InputText("##CameraName", inputText, IM_ARRAYSIZE(inputText));

			if (ImGui::Button("OK")) {
				for (auto& entity : cameraEntities) {
					entity.getComponent<CameraMatrixComponent>().primary = false;
				}
				pm_scene->add_camMat_entt(std::string(inputText), type_, false);
				selectedIndex = (int)cameraNames.size() - 1;
				showCameraNamePopup = false;
				ImGui::CloseCurrentPopup();

			}
			ImGui::EndPopup();
		}
	}

	bool MainWindow::fileHandler_shortcut(KeyPressedEvent& keyEvent)
	{
		if (keyEvent.getRepeatCount() > 0) {
			return false;
		}
		bool pressedCtrls = Input::IsKeyPressed(WMS_KEY_LEFT_CONTROL) || Input::IsKeyPressed(WMS_KEY_RIGHT_CONTROL);
		bool pressedShift = Input::IsKeyPressed(WMS_KEY_LEFT_SHIFT) || Input::IsKeyPressed(WMS_KEY_RIGHT_SHIFT);
		switch (keyEvent.getKeyCode()) {
		case WMS_KEY_S: {
			if (pressedCtrls && pressedShift) {
				saveAsScene();
			}
			break;
		}
		case WMS_KEY_O: {
			if (pressedCtrls) {
				openScene();
			}
			break;
		}
		case WMS_KEY_N: {
			if (pressedCtrls) {
				newScene();
			}
			break;
		}
		}
	}
	void MainWindow::openScene()
	{
		const std::string& filePath = FileDialog::openFile("WORKMANs Scene (*.wmsa)\0*.wmsa\0");
		if (!filePath.empty()) {
			pm_scene = std::make_shared<Scene>();
			pm_scene->resizeView((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
			pm_sceneProperties_panel.setScene(pm_scene);
			pm_scene->onAttach();
			Serialize dataHandler(pm_scene);
			dataHandler.deserialize(filePath);
		}
	}

	void MainWindow::saveAsScene()
	{
		const std::string& filePath = FileDialog::saveFile("WORKMANs Scene (*.wmsa)\0*.wmsa\0");
		if (!filePath.empty()) {
			Serialize dataHandler(pm_scene);
			dataHandler.serialize(filePath);
		}

	}

	void MainWindow::newScene()
	{
		pm_scene = std::make_shared<Scene>();
		pm_sceneProperties_panel.setScene(pm_scene);
		pm_scene->onAttach();
	}



}
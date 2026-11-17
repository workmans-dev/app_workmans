#include "pch.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/imgui.h"

#include "debugs/instrumentation.h"
#include "mainWindow.h"

#include "dataHandler/sceneSerialize.h"
#include "dataHandler/fileHandler/fileDialog.h"

#include "renderer/geometry/cube.h"
#include "vulkan/vk_renderTarget.h"   // for the getRenderPass() downcast
#include "vulkan/vulkanContext.h"     // borrows the current command buffer


#include "ImGuizmo/ImGuizmo.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>


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


		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;


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
				// The label is not hardcoded -- change a key in keymap.yaml and the menu display changes with it.
				// This is why a declarative file was chosen: with a script, "what is the current shortcut"
				// would require running code, which makes this display impossible.
				ImGui::Separator();
				if (ImGui::MenuItem("New Scene", KeyMap::label("file.newScene"), false, dockSpace_open != NULL)) {
					newScene();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Save As", KeyMap::label("file.saveAsScene"), false, dockSpace_open != NULL)) {
					saveAsScene();
				}
				if (ImGui::MenuItem("Open Scene", KeyMap::label("file.openScene"), false, dockSpace_open != NULL)) {
					openScene();
				}
				if (ImGui::MenuItem("Exit", KeyMap::label("app.exit"), false, dockSpace_open != NULL)) {
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
							// A primitive is a thing that shows up in the viewport, so it is an object.
							// (Position-less things such as modifiers go through createEntity instead)
							Entity entt = pm_scene->createObject("Cube");
							entt.addComponent<MeshComponent>(Cube::create());
						}
						if (ImGui::MenuItem("Plane", NULL, false, dockSpace_open != NULL)) {
							Entity entt = pm_scene->createObject("Plane1");
							entt.addComponent<ShadingComponent>(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
							std::cout << "added components: ShadingComponent" << std::endl;
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
		
		// onUpdate already finished recording into the offscreen FB (App::run order: onUpdate -> onImGuiRender).
		// getColorTextureID() is really a VkDescriptorSet, the same width as ImTextureID (= ImU64).
		ImGui::Image((ImTextureID)pm_viewportTarget->getColorTextureID(),
			ImVec2{ pm_viewportSize.x, pm_viewportSize.y });

		// WARNING: IsItemHovered has to come right after Image() -- drawing another item moves
		//    "the last item" over to that one (the same trap as the outliner tree).
		const bool imageHovered = ImGui::IsItemHovered();

		// The actual rectangle the image just drawn occupies on screen.
		//    Reading it here rather than hardcoding the toolbar height keeps it correct when padding or the toolbar changes.
		const ImVec2 imageMin = ImGui::GetItemRectMin();
		const ImVec2 imageMax = ImGui::GetItemRectMax();

		// What the gizmo attaches to = the one picked last. It is the same condition as the ImGuizmo
		// block below, so it is computed once here and shared by both.
		Entity* activeEntity = pm_sceneProperties_panel.get_activeEntity();
		const bool gizmoDrawn = (activeEntity && pm_manipulatorType != -1);

		// -- P-b picking request ---------------------------------------------
		// Screen coordinates -> texel relative to the image's top-left. The image is drawn at 1:1
		// (resize follows the panel size directly), so no scale correction is needed.
		//
		// WARNING: nothing is picked over the gizmo. Mistaking a press meant to drag an axis for a
		//    selection would change the target mid-manipulation.
		// WARNING: but ask only *when the gizmo is actually drawn*. ImGuizmo::IsOver() computes the
		//    intersection from the matrix last frame's Manipulate() left in gContext, and with no
		//    selection Manipulate was never called, so that matrix is zero.
		//    Asking anyway can come back true spuriously and *swallow the first click*.
		// WARNING: nothing is picked during camera manipulation. camera.modifier (LeftAlt by default)
		//    + left click is already tumble, so taking a press meant to orbit as a selection would
		//    wipe the whole selection the moment the mouse goes down.
		//    The key is not hardcoded -- it reads whatever key keymap.yaml set.
		const bool cameraDragging = KeyMap::isHeld("camera.modifier");

		if (imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& !cameraDragging
			&& !(gizmoDrawn && (ImGuizmo::IsOver() || ImGuizmo::IsUsing()))) {
			const ImVec2 mouse = ImGui::GetMousePos();
			const int px = (int)(mouse.x - imageMin.x);
			const int py = (int)(mouse.y - imageMin.y);
			if (px >= 0 && py >= 0) {
				// The modifiers are read *now* -- the result arrives a few frames later.
				pm_pendingPickMode = SceneProperties::selectModeFromMods();
				// Attachment 1 = the id buffer (the order listed in onAttach)
				pm_viewportTarget->requestPixelRead(1, (uint32_t)px, (uint32_t)py);
			}
		}

		// ImGuizmo  (activeEntity / gizmoDrawn were already computed in the picking block above)
		if (gizmoDrawn) {
			// primary cam_entity
			auto* camEntity_primary = pm_scene->get_camera_primary();
			auto& sceneCam = camEntity_primary->getComponent<CameraMatrixComponent>().sceneCam;

			ImGuizmo::SetOrthographic(
				sceneCam.get_camProjType() == SceneCamera::CameraProjType::orthographic);

			// WARNING: the default is true, which makes the axis arrows flip around as the camera moves.
			//    ImGuizmo compares the screen-projected length of the + and - axes (ImGuizmo.cpp:1202)
			//    and turns whichever faces the camera more -- a "visibility improvement" feature, but
			//    in a DCC the axis direction must be fixed for the manipulation direction to be predictable.
			//    Maya's manipulator is also always pinned to the positive axes.
			ImGuizmo::AllowAxisFlip(false);

			ImGuizmo::SetDrawlist();

			// Fits the gizmo coordinate space exactly to the 3D image area
			ImGuizmo::SetRect(imageMin.x, imageMin.y,
				imageMax.x - imageMin.x, imageMax.y - imageMin.y);

			// WARNING: this is the original (OpenGL NDC). Handing it the get_projection_vk() that
			//    rendering uses makes ImGuizmo apply its own Y flip (ImGuizmo.cpp:819) again and reverse the direction.
			const glm::mat4& cam_proj = sceneCam.get_projection();
			// The view is the inverse of the camera's *world*. It has to be the same value as the render
			// path in Scene::onUpdate, or the gizmo will not line up with where things are drawn.
			// (onImGuiRender comes after onUpdate, so updateTransforms() has already run)
			glm::mat4 cam_transform = glm::inverse(pm_scene->getWorldMatrix(*camEntity_primary));

			// What the gizmo attaches to = *the one picked last*. The same rule as Maya's manipulator,
			// and it matches what the Property editor shows.
			Entity& selectedEntity = *activeEntity;

			// WARNING: the gizmo is drawn in *world space* -- it takes view/proj and projects this
			//    matrix to the screen as is. Handing it the local would draw the gizmo of a parented
			//    object somewhere away from the object, and dragging would jump by the parent transform.
			//    (On an object with no parent the two are identical, so the bug stays invisible)
			glm::mat4 transform = pm_scene->getWorldMatrix(selectedEntity);

			ImGuizmo::Manipulate(glm::value_ptr(cam_transform), glm::value_ptr(cam_proj),
				(ImGuizmo::OPERATION)pm_manipulatorType, ImGuizmo::LOCAL, glm::value_ptr(transform));


			if (ImGuizmo::IsUsing()) {
				// What comes back is the *new world*. Scene is what back-solves the local so that world
				// comes out under the parent -- it shares the same formula as setParent.
				pm_scene->setWorldMatrix(selectedEntity, transform);
			}

			
		}


		ImGui::PopStyleVar();
		ImGui::End();
	}

	void MainWindow::onAttach() {

		// The shortcuts are decided by a file, not by the code. The path is relative to the working
		// directory, the same rule as the shaders (assets/shaders). With no file it starts on the defaults.
		KeyMap::load("assets/config/keymap.yaml");

		// WARNING: by this point App::App() has finished and the ImGui Vulkan backend is up.
		//    (the pushLayer in Sandbox's body comes after the base class constructor) -> AddTexture is safe.
		RenderTargetSpec targetSpec;
		targetSpec.w = (uint32_t)pm_viewportSize.x;
		targetSpec.h = (uint32_t)pm_viewportSize.y;
		// The order listed = attachment slot = the out location in cube.frag.
		// Slot 1 (R32SINT) is the entity id buffer used for picking.
		targetSpec.attachments = { AttachmentFormat::RGBA8,
								   AttachmentFormat::R32SINT,
								   AttachmentFormat::Depth };

		pm_viewportTarget = RenderTarget::create(targetSpec);

		// WARNING: forced order -- the pipeline targets the FB's render pass, so it comes after the FB.
		// The vertex data (VertexArray) is owned by Scene; only the layout is needed here.
		PipelineSpec pipeSpec;
		pipeSpec.vertPath = "assets/shaders/cube.vert.spv";
		pipeSpec.fragPath = "assets/shaders/cube.frag.spv";
		pipeSpec.renderPass =
			static_cast<VulkanRenderTarget*>(pm_viewportTarget.get())->getRenderPass();
		pipeSpec.vertexLayout = Cube::layout();
		// u_MVP + u_Model + u_EntityID from cube.vert
		pipeSpec.pushConstantSize = sizeof(glm::mat4) * 2 + sizeof(int32_t);
		// WARNING: u_EntityID is read by the frag -- leaving only VERTEX on gets caught by validation.
		pipeSpec.pushConstantStages =
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// WARNING: must equal the subpass colorAttachmentCount (2, with the id buffer attached).
		pipeSpec.colorAttachmentCount =
			static_cast<VulkanRenderTarget*>(pm_viewportTarget.get())->getColorAttachmentCount();
		// cullMode/frontFace use the PipelineSpec defaults (BACK + CCW). See the comments there for why.

		pm_cubePipeline = std::make_unique<VulkanPipeline>(pipeSpec);

		pm_scene = std::make_shared<Scene>();
		pm_sceneProperties_panel.setScene(pm_scene);
		pm_scene->onAttach();

	}

	void MainWindow::onUpdate(AppWMs::Timer time)
	{
		APP_WMS_PROFILE_FUNC();

		// The panel size is refreshed in onImGuiRender (setViewport), so here it is one frame behind.
		// An unchanged size is ignored inside resize(), so calling it every frame is fine.
		if (pm_viewportSize.x > 0.0f && pm_viewportSize.y > 0.0f) {
			pm_viewportTarget->resize((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
			pm_scene->resizeView((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
		}

		{
			SCOPE_PROFILE_TIMER("Rendering on update");
			// WARNING: mind the order -- on Vulkan the vkCmdBeginRenderPass in bind() clears with
			//    loadOp=CLEAR and reads clearValue right then. So setClearColor has to come before bind().
			//    (On OpenGL it was glClear after bind, so the order was the other way round)
			RendererCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
			RendererCommand::clear();   // a no-op on Vulkan

			VkCommandBuffer cmd = VulkanContext::Get()->getCurrentCommandBuffer();

			pm_viewportTarget->beginPass();
			// With only one pipeline, it is bound once right after the render pass opens.
			// As pipelines grow this moves out of here into a draw sorting loop (the purpose of decision #4 C).
			pm_cubePipeline->bind(cmd);

			// The draws are issued by Scene::onUpdate through Renderer::submit, with the camera matrices.
			pm_scene->onUpdate(time);
			pm_viewportTarget->endPass();

			// -- P-c -- applies a ripened picking result to the selection.
			//    Coming out a few frames after the request (onImGuiRender) is normal.
			//
			// -1 becomes entt::null as is (the id buffer's clear value is all bits 1 and
			//    entt::null is all bits 1 too, so the conversion is free) -> a click on empty
			//    space flows straight through as "no selection".
			int32_t picked = -1;
			if (pm_viewportTarget->pollPixelRead(picked)) {
				pm_sceneProperties_panel.applyPickSelection(
					(entt::entity)(uint32_t)picked, pm_pendingPickMode);
			}
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
		// Changed from switch(keycode) + hand-checking modifiers to an action lookup.
		// Which key it is is not here -- keymap.yaml decides.
		//
		// WARNING: KeyMap::isAction is true only when the modifiers match *exactly*.
		//    That is why S and Ctrl+Shift+S do not swallow each other.
		const int key = keyEvent.getKeyCode();

		if (KeyMap::isAction("file.newScene", key))          newScene();
		else if (KeyMap::isAction("file.openScene", key))    openScene();
		else if (KeyMap::isAction("file.saveAsScene", key))  saveAsScene();
		else if (KeyMap::isAction("app.exit", key)) {
			// The menu had long shown Ctrl+Q with no actual handling. This makes display and behavior agree.
			dockSpace_open = false;
			App::getApp().close();
		}
		else if (KeyMap::isAction("gizmo.none", key))        pm_manipulatorType = 0;
		else if (KeyMap::isAction("gizmo.translate", key))   pm_manipulatorType = ImGuizmo::OPERATION::TRANSLATE;
		else if (KeyMap::isAction("gizmo.rotate", key))      pm_manipulatorType = ImGuizmo::OPERATION::ROTATE;
		else if (KeyMap::isAction("gizmo.scale", key))       pm_manipulatorType = ImGuizmo::OPERATION::SCALE;
		// Outliner/selection shortcuts are judged by the panel itself (delete, visibility, later parent).
		else if (pm_sceneProperties_panel.onKeyPressed(key)) { /* the panel handled it */ }
		else return false;   // not ours -> pass it to the next layer (it used to return true unconditionally)

		return true;
	}

	void MainWindow::openScene()
	{
		const std::string& filePath = FileDialog::openFile("WORKMANs Scene (*.wmsa)\0*.wmsa\0");
		if (!filePath.empty()) {
			pm_scene = std::make_shared<Scene>();
			pm_sceneProperties_panel.setScene(pm_scene);
			pm_scene->onAttach();
			pm_scene->resizeView((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);

			Serialize dataHandler(pm_scene);
			dataHandler.deserialize(filePath);


			std::vector<Entity>& camEntities = pm_scene->get_camEntities();
			std::vector<std::string>& cameraNames = pm_scene->get_cameraNames();

			selectedIndex = 0; // Default to first camera if no defaultCam is found
			for (size_t i = 0; i < camEntities.size(); i++) {
				if (camEntities[i].getComponent<CameraMatrixComponent>().primary) {
					selectedIndex = static_cast<int>(i);
					break;
				}
			}

			// Ensure the correct camera is set as primary
			for (auto& entity : camEntities) {
				entity.getComponent<CameraMatrixComponent>().primary = false;
			}
			camEntities[selectedIndex].getComponent<CameraMatrixComponent>().primary = true;
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
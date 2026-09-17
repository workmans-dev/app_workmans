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
#include "vulkan/vk_renderTarget.h"   // getRenderPass() 다운캐스팅용
#include "vulkan/vulkanContext.h"     // 현재 커맨드버퍼 대여


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
				// 라벨을 하드코딩하지 않는다 — keymap.yaml에서 키를 바꾸면 메뉴 표시도 같이 바뀐다.
				// 선언적 파일을 고른 이유가 이것이다: 스크립트였으면 "현재 단축키가 뭔지"를
				// 알아내려고 코드를 실행해야 해서 이 표시가 불가능하다.
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
							// 프리미티브는 뷰포트에 드러나는 물건이므로 오브젝트다.
							// (모디파이어처럼 위치 없는 것은 createEntity 쪽으로 간다)
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
		
		// onUpdate에서 이미 오프스크린 FB에 기록이 끝났다 (App::run 순서: onUpdate → onImGuiRender).
		// getColorTextureID()의 실체는 VkDescriptorSet, ImTextureID(=ImU64)와 폭이 같다.
		ImGui::Image((ImTextureID)pm_viewportTarget->getColorTextureID(),
			ImVec2{ pm_viewportSize.x, pm_viewportSize.y });

		// ⚠️ IsItemHovered는 Image() 바로 뒤여야 한다 — 다른 아이템을 그리면
		//    "마지막 아이템"이 그쪽으로 넘어간다 (아웃라이너 트리와 같은 함정).
		const bool imageHovered = ImGui::IsItemHovered();

		// ⭐️ 방금 그린 이미지가 화면에서 차지한 실제 사각형.
		//    툴바 높이를 상수로 빼는 대신 여기서 읽으면 패딩·툴바가 바뀌어도 안 어긋난다.
		const ImVec2 imageMin = ImGui::GetItemRectMin();
		const ImVec2 imageMax = ImGui::GetItemRectMax();

		// 기즈모가 붙는 대상 = 마지막에 고른 것. 아래 ImGuizmo 블록과 같은 조건이라
		// 여기서 한 번 구해 둘이 공유한다.
		Entity* activeEntity = pm_sceneProperties_panel.get_activeEntity();
		const bool gizmoDrawn = (activeEntity && pm_manipulatorType != -1);

		// ── P-b 피킹 요청 ────────────────────────────────────────────────────
		// 화면 좌표 → 이미지 좌상단 기준 텍셀. 이미지를 1:1 크기로 그리므로
		// (resize가 패널 크기를 그대로 따라간다) 배율 보정이 필요 없다.
		//
		// ⚠️ 기즈모 위에서는 안 잡는다. 축을 끌려고 누른 것을 선택으로 오해하면
		//    조작 중에 대상이 바뀐다.
		// ⚠️ 단, **기즈모가 실제로 그려질 때만** 물어본다. ImGuizmo::IsOver()는
		//    지난 프레임 Manipulate()가 gContext에 남긴 행렬로 교차를 계산하는데,
		//    선택이 없으면 Manipulate가 한 번도 안 불려 그 행렬이 0이다.
		//    그대로 물어보면 엉뚱하게 참이 나와 **첫 클릭이 삼켜질 수 있다.**
		// ⚠️ 카메라 조작 중에는 안 잡는다. camera.modifier(기본 LeftAlt) + 좌클릭은
		//    이미 tumble이라, 궤도를 돌리려고 누른 것을 선택으로 받으면
		//    마우스를 누르는 순간 선택이 통째로 날아간다.
		//    키를 하드코딩하지 않는다 — keymap.yaml이 정한 그 키를 그대로 본다.
		const bool cameraDragging = KeyMap::isHeld("camera.modifier");

		if (imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& !cameraDragging
			&& !(gizmoDrawn && (ImGuizmo::IsOver() || ImGuizmo::IsUsing()))) {
			const ImVec2 mouse = ImGui::GetMousePos();
			const int px = (int)(mouse.x - imageMin.x);
			const int py = (int)(mouse.y - imageMin.y);
			if (px >= 0 && py >= 0) {
				// 조합키는 **지금** 읽어둔다 — 결과는 몇 프레임 뒤에 온다.
				pm_pendingPickMode = SceneProperties::selectModeFromMods();
				// 어태치먼트 1번 = id 버퍼 (onAttach의 나열 순서)
				pm_viewportTarget->requestPixelRead(1, (uint32_t)px, (uint32_t)py);
			}
		}

		// ImGuizmo  (activeEntity / gizmoDrawn은 위 피킹 블록에서 이미 구했다)
		if (gizmoDrawn) {
			// primary cam_entity
			auto* camEntity_primary = pm_scene->get_camera_primary();
			auto& sceneCam = camEntity_primary->getComponent<CameraMatrixComponent>().sceneCam;

			ImGuizmo::SetOrthographic(
				sceneCam.get_camProjType() == SceneCamera::CameraProjType::orthographic);

			// ⚠️ 기본값이 true라 카메라를 돌리면 축 화살표가 제멋대로 뒤집힌다.
			//    ImGuizmo가 +축/−축의 화면 투영 길이를 비교해(ImGuizmo.cpp:1202)
			//    카메라를 더 마주보는 쪽으로 돌려놓는 "가시성 개선" 기능인데,
			//    DCC에선 축 방향이 고정이어야 조작 방향을 예측할 수 있다.
			//    마야 매니퓰레이터도 항상 양의 축에 고정이다.
			ImGuizmo::AllowAxisFlip(false);

			ImGuizmo::SetDrawlist();

			// 기즈모 좌표계를 3D 이미지 영역에 정확히 맞춘다
			ImGuizmo::SetRect(imageMin.x, imageMin.y,
				imageMax.x - imageMin.x, imageMax.y - imageMin.y);

			// ⚠️ 여기는 원본(OpenGL NDC)이다. 렌더가 쓰는 get_projection_vk()를 주면
			//    ImGuizmo가 자체 Y 뒤집기(ImGuizmo.cpp:819)를 또 해서 방향이 반대가 된다.
			const glm::mat4& cam_proj = sceneCam.get_projection();
			// 뷰는 카메라의 **월드** 역행렬이다. Scene::onUpdate의 렌더 경로와 같은 값을
			// 써야 기즈모가 화면에 그려진 위치와 어긋나지 않는다.
			// (onImGuiRender는 onUpdate 뒤라 updateTransforms()가 이미 돌았다)
			glm::mat4 cam_transform = glm::inverse(pm_scene->getWorldMatrix(*camEntity_primary));

			// 기즈모가 붙는 대상 = **마지막에 고른 것**. 마야 매니퓰레이터와 같은 규칙이고,
			// Property editor가 보여주는 대상과도 일치한다.
			Entity& selectedEntity = *activeEntity;

			// ⚠️ 기즈모는 **월드 공간**에서 그린다 — view/proj를 받아 이 행렬을 그대로
			//    화면에 투영한다. 로컬을 주면 부모가 있는 오브젝트의 기즈모가 물체에서
			//    떨어진 엉뚱한 자리에 그려지고, 끌면 부모 변환만큼 튄다.
			//    (부모가 없는 오브젝트에선 둘이 같아서 이 버그가 안 보인다)
			glm::mat4 transform = pm_scene->getWorldMatrix(selectedEntity);

			ImGuizmo::Manipulate(glm::value_ptr(cam_transform), glm::value_ptr(cam_proj),
				(ImGuizmo::OPERATION)pm_manipulatorType, ImGuizmo::LOCAL, glm::value_ptr(transform));


			if (ImGuizmo::IsUsing()) {
				// 돌려받은 건 **새 월드**다. 부모 밑에서 그 월드가 나오도록 로컬을
				// 역산해 넣는 건 Scene이 한다 — setParent와 같은 식을 공유한다.
				pm_scene->setWorldMatrix(selectedEntity, transform);
			}

			
		}


		ImGui::PopStyleVar();
		ImGui::End();
	}

	void MainWindow::onAttach() {

		// 단축키는 코드가 아니라 파일이 정한다. 경로는 실행 디렉토리 기준이고,
		// 셰이더(assets/shaders)와 같은 규칙이다. 파일이 없으면 내장 기본값으로 뜬다.
		KeyMap::load("assets/config/keymap.yaml");

		// ⚠️ 이 시점엔 App::App()이 이미 끝나 ImGui 벌칸 백엔드가 올라와 있다.
		//    (Sandbox 본문의 pushLayer는 기반 클래스 생성자 이후) → AddTexture 호출 안전.
		RenderTargetSpec targetSpec;
		targetSpec.w = (uint32_t)pm_viewportSize.x;
		targetSpec.h = (uint32_t)pm_viewportSize.y;
		// 나열 순서 = 어태치먼트 슬롯 = cube.frag의 out location.
		// 슬롯 1(R32SINT)이 피킹용 엔티티 id 버퍼다.
		targetSpec.attachments = { AttachmentFormat::RGBA8,
								   AttachmentFormat::R32SINT,
								   AttachmentFormat::Depth };

		pm_viewportTarget = RenderTarget::create(targetSpec);

		// ⚠️ 순서 강제 — 파이프라인은 FB의 렌더패스를 대상으로 삼으므로 FB보다 뒤여야 한다.
		// 정점 데이터(VertexArray)는 Scene이 소유하고, 여기선 레이아웃만 필요하다.
		PipelineSpec pipeSpec;
		pipeSpec.vertPath = "assets/shaders/cube.vert.spv";
		pipeSpec.fragPath = "assets/shaders/cube.frag.spv";
		pipeSpec.renderPass =
			static_cast<VulkanRenderTarget*>(pm_viewportTarget.get())->getRenderPass();
		pipeSpec.vertexLayout = Cube::layout();
		// cube.vert의 u_MVP + u_Model + u_EntityID
		pipeSpec.pushConstantSize = sizeof(glm::mat4) * 2 + sizeof(int32_t);
		// ⚠️ u_EntityID는 frag가 읽는다 — VERTEX만 켜두면 validation이 잡는다.
		pipeSpec.pushConstantStages =
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// ⚠️ subpass의 colorAttachmentCount와 반드시 같아야 한다 (id 버퍼가 붙어 2개).
		pipeSpec.colorAttachmentCount =
			static_cast<VulkanRenderTarget*>(pm_viewportTarget.get())->getColorAttachmentCount();
		// cullMode/frontFace는 PipelineSpec 기본값(BACK + CCW)을 쓴다. 근거는 그쪽 주석 참고.

		pm_cubePipeline = std::make_unique<VulkanPipeline>(pipeSpec);

		pm_scene = std::make_shared<Scene>();
		pm_sceneProperties_panel.setScene(pm_scene);
		pm_scene->onAttach();

	}

	void MainWindow::onUpdate(AppWMs::Timer time)
	{
		APP_WMS_PROFILE_FUNC();

		// 패널 크기는 onImGuiRender(setViewport)에서 갱신되므로 여기선 한 프레임 뒤처진 값이다.
		// 같은 크기면 resize() 내부에서 무시되므로 매 프레임 불러도 된다.
		if (pm_viewportSize.x > 0.0f && pm_viewportSize.y > 0.0f) {
			pm_viewportTarget->resize((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
			pm_scene->resizeView((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
		}

		{
			SCOPE_PROFILE_TIMER("Rendering on update");
			// ⚠️ 순서 주의 — 벌칸은 bind()의 vkCmdBeginRenderPass가 loadOp=CLEAR로 지우고,
			//    그때 clearValue를 읽는다. 따라서 setClearColor가 bind()보다 먼저여야 한다.
			//    (OpenGL은 bind 후 glClear라 순서가 반대였음)
			RendererCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
			RendererCommand::clear();   // 벌칸에선 no-op

			VkCommandBuffer cmd = VulkanContext::Get()->getCurrentCommandBuffer();

			pm_viewportTarget->beginPass();
			// 파이프라인 1개뿐이라 렌더패스 개시 직후 한 번 바인딩한다.
			// 파이프라인이 늘면 여기가 아니라 드로우 정렬 루프로 옮겨간다 (④ C안의 목적).
			pm_cubePipeline->bind(cmd);

			// 드로우는 Scene::onUpdate가 카메라 행렬과 함께 Renderer::submit으로 낸다.
			pm_scene->onUpdate(time);
			pm_viewportTarget->endPass();

			// ── P-c — 익은 피킹 결과를 선택에 반영한다.
			//    요청(onImGuiRender)과 몇 프레임 떨어져 나오는 게 정상이다.
			//
			// 🟢 -1은 그대로 entt::null이 된다 (id 버퍼의 clear 값이 전 비트 1이고
			//    entt::null도 전 비트 1이라 변환이 공짜다) → 빈 곳 클릭이 그대로
			//    "선택 없음"으로 흘러간다.
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
		// switch(키코드) + 조합키 손검사에서 액션 조회로 바꿨다.
		// 어떤 키인지는 여기 없다 — keymap.yaml이 정한다.
		//
		// ⚠️ KeyMap::isAction은 조합키가 **정확히 일치**할 때만 참이다.
		//    그래서 S와 Ctrl+Shift+S가 서로를 잡아먹지 않는다.
		const int key = keyEvent.getKeyCode();

		if (KeyMap::isAction("file.newScene", key))          newScene();
		else if (KeyMap::isAction("file.openScene", key))    openScene();
		else if (KeyMap::isAction("file.saveAsScene", key))  saveAsScene();
		else if (KeyMap::isAction("app.exit", key)) {
			// 메뉴가 예전부터 Ctrl+Q를 표시했는데 실제 처리는 없었다. 표시와 동작을 맞춘다.
			dockSpace_open = false;
			App::getApp().close();
		}
		else if (KeyMap::isAction("gizmo.none", key))        pm_manipulatorType = 0;
		else if (KeyMap::isAction("gizmo.translate", key))   pm_manipulatorType = ImGuizmo::OPERATION::TRANSLATE;
		else if (KeyMap::isAction("gizmo.rotate", key))      pm_manipulatorType = ImGuizmo::OPERATION::ROTATE;
		else if (KeyMap::isAction("gizmo.scale", key))       pm_manipulatorType = ImGuizmo::OPERATION::SCALE;
		// 아웃라이너/선택 관련 단축키는 패널이 직접 판단한다 (삭제·가시성, 이후 parent).
		else if (pm_sceneProperties_panel.onKeyPressed(key)) { /* 패널이 처리했다 */ }
		else return false;   // 우리 게 아니면 다음 레이어로 흘려보낸다 (예전엔 무조건 true였다)

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
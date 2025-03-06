#include "pch.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/imgui.h"

#include "debugs/instrumentation.h"
#include "viewport.h"

namespace AppWMs {

	Viewport::Viewport() : Layer("viewport") {
        
	}       

	void Viewport::onAttach(){

        FrameBufferSpec FB_spec = {1280, 720};
        pm_frameBuffer = FrameBuffer::create(FB_spec);

        pm_scene = std::make_shared<Scene>();

        g_sq_entt = pm_scene->createEntity("blue squre");
        g_sq_entt.addComponent<ShadingComponent>(glm::vec4(0.2f, 0.3f, 0.8f, 1.0f ));
        r_sq_entt = pm_scene->createEntity("red squre");
        r_sq_entt.addComponent<ShadingComponent>(glm::vec4(0.8f, 0.0f, 0.1f, 1.0f));

        add_camMat_entt("perspective", SceneCamera::CameraProjType::perspective);
        add_camMat_entt("front", SceneCamera::CameraProjType::orthographic);

        pm_sceneProperties_panel.setScene(pm_scene);
	}

	void Viewport::onUpdate(AppWMs::Timer time)
	{
        APP_WMS_PROFILE_FUNC();
        const FrameBufferSpec& f_spec = pm_frameBuffer->get_spec();
        if (pm_viewportSize != glm::vec2(f_spec.w, f_spec.h)) {
            pm_frameBuffer->resize((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
            //pm_cameraController.resizeRatio((uint32_t)pm_viewportSize.x, (uint32_t)pm_viewportSize.y);
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

	void Viewport::onEvent(AppWMs::Event& event)
	{
		//pm_cameraController.onEvent(event);
	}

	void Viewport::onImGuiRender()
	{
        static bool dockSpace_open = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
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
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockSpace_open, window_flags);

        if (!opt_padding)
            ImGui::PopStyleVar();
        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", NULL, false, dockSpace_open != NULL)) {
                    dockSpace_open = false;
                    App::getApp().close();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();
        
        //create propery editor! it will show components of the entity(property)
        pm_sceneProperties_panel.onImGuiRender();

        // ----- view port -------
        ImGui::Begin("Viewport");

        // tool bar
        createToolBar();

        const bool& fact_hovered = ImGui::IsWindowHovered();
        const bool& fact_focus = ImGui::IsWindowFocused();
        // event factor
        pm_viewportEventBlocking =  !fact_hovered;
        App::getApp().get_ImGuiLayer()->blockEvent(pm_viewportEventBlocking);
        pm_scene->set_block_onUpdate(pm_viewportEventBlocking);
        const ImVec2& resized_viewportSize = ImGui::GetContentRegionAvail();
        const float toolbarHeight = 20.0f;
        pm_viewportSize = { resized_viewportSize.x, resized_viewportSize.y };
        uint32_t textureID = pm_frameBuffer->get_colorAttach();
        ImGui::Image(textureID, ImVec2{ pm_viewportSize.x, pm_viewportSize.y }, ImVec2{0,1}, ImVec2{ 1,0 });


        ImGui::End();


	}

    void Viewport::add_camMat_entt(const std::string& entt_tagName, SceneCamera::CameraProjType type)
    {
        Entity camMat_entt = pm_scene->createEntity(entt_tagName);
        camMat_entt.addComponent<CameraMatrixComponent>();
        camMat_entt.addComponent<NativeScriptComponent>().bind<CameraController>();
        pm_cam_entities.emplace_back(camMat_entt);
        // set cam projtion type!!!
        camMat_entt.getComponent<CameraMatrixComponent>().sceneCam.set_camProjType(type);
        pm_cameraViewArray.emplace_back(entt_tagName);
        if (type == SceneCamera::CameraProjType::orthographic)
            camMat_entt.getComponent<CameraMatrixComponent>().primary = false;
    }

    void Viewport::createToolBar()
    {
        // Define the toolbar and combo box sizes
        const float toolbarHeight = 14.0f;
        const float comboBoxWidth = 300.0f;
        // Create the toolbar container
        ImVec2 toolbarSize = ImVec2(ImGui::GetContentRegionAvail().x, toolbarHeight);
        ImGui::BeginChild("Toolbar", toolbarSize, ImGuiChildFlags_AutoResizeX|ImGuiChildFlags_AutoResizeY, 
            ImGuiWindowFlags_NoScrollWithMouse| ImGuiWindowFlags_NoScrollbar);
        // Render text
        ImGui::Text("Camera:");
        ImGui::SameLine();

        // Create the combo box
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // Remove padding

        if (ImGui::BeginCombo("##CameraCombo", pm_cameraViewArray[selectedIndex].c_str(), 
            ImGuiComboFlags_HeightSmall)) {
            std::cout << "isSelected" << std::endl;
            for (int i = 0; i < pm_cam_entities.size(); i++) {
                auto& camComp = pm_cam_entities[i].getComponent<CameraMatrixComponent>();
                if (i == 1) {
                    ImGui::TextDisabled("Orthographic"); // Non-selectable header
                    ImGui::SameLine();
                    float cursorY = ImGui::GetCursorPosY();
                    ImGui::SetCursorPosY(cursorY + 5.0f); // Small vertical alignment
                    ImGui::Separator(); // Default horizontal separator
                }

                bool isSelected = (selectedIndex == i)?true:false;
                std::cout << isSelected << std::endl;

                if (ImGui::Selectable(pm_cameraViewArray[i].c_str(), isSelected)) {
                    selectedIndex = i;
                    
                    for (auto& entity : pm_cam_entities) {
                        entity.getComponent<CameraMatrixComponent>().primary = false;
                    }
                    // Now set the primary flag for the selected camera
                    pm_cam_entities[selectedIndex].getComponent<CameraMatrixComponent>().primary=true;
                    std::cout << selectedIndex << ": selected" << std::endl;
                    std::cout << pm_cam_entities[selectedIndex].getComponent<TagComponent>().name << ": camName" << std::endl;
                    
                }
            }

            ImGui::EndCombo();
        }
        ImGui::PopStyleVar();
        ImGui::EndChild(); // End toolbar
    }
}

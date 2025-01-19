#include "app_workmans.h"
#include "imgui/imgui.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

class WindowLayer:public AppWMs::Layer
{
public:
	WindowLayer() :Layer("Example"), pm_camera(-1.6f, 1.6f, -.9f, .9f), pm_camera_pos(0.0f){
		// test for triangle
		pm_vertexArray_ptr.reset(AppWMs::VertexArray::create());

		float tri_verts[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};

		float sq_verts[3 * 4] = {
			-0.5f, -0.5f, 0.0f, 
			0.5f, -0.5f, 0.0f, 
			0.5f, 0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f,
		};
		std::shared_ptr<AppWMs::VertexBuffer> pm_vertexBuffer_ptr;
		pm_vertexBuffer_ptr.reset(AppWMs::VertexBuffer::create(sq_verts, sizeof(sq_verts)));

		AppWMs::BufferLayout layout = {
			// these elements are the below, 
			// BufferElements(ShaderDataType type, const std::string& name, bool normalized =false)
			{AppWMs::ShaderDataType::Float3, "a_Position"}
			//{AppWMs::ShaderDataType::Float4, "v_Color"}
		};
		pm_vertexBuffer_ptr->setLayout(layout);
		pm_vertexArray_ptr->addVertexBuffer(pm_vertexBuffer_ptr);

		unsigned int tri_indexes[3] = { 0,1,2 };
		unsigned int sq_indexes[6] = { 0,1,2,2,3,0 };
		std::shared_ptr<AppWMs::IndexBuffer> pm_indexBuffer_ptr;
		pm_indexBuffer_ptr.reset(AppWMs::IndexBuffer::create(sq_indexes, sizeof(sq_indexes) / sizeof(uint32_t)));
		pm_vertexArray_ptr->setIndexBuffer(pm_indexBuffer_ptr);

		// shader
		std::string tri_verts_src = R"(
			#version 410 core
			layout(location=0) in vec3 a_Position;
			layout(location=1) in vec4 a_Color;

			uniform mat4 u_view_proj;
			uniform mat4 u_transform;

			out vec3 v_Position;
			out vec4 v_Color;
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_view_proj * u_transform * vec4(a_Position, 1.0);
			}
		)";
		std::string tri_frag_src = R"(
			#version 410 core
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;
			void main()
			{
				color = v_Color;
			}
		)";
		std::string sq_verts_src = R"(
			#version 410 core
			layout(location=0) in vec3 a_Position;

			uniform mat4 u_view_proj;
			uniform mat4 u_transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_view_proj * u_transform * vec4(a_Position*0.5+0.5, 1.0);
			}
		)";
		std::string sq_frag_src = R"(
			#version 410 core
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			uniform vec4 v_Color;
			
			void main()
			{
				color = v_Color;
			}
		)";

		pm_shader_ptr.reset(new AppWMs::Shader(sq_verts_src, sq_frag_src));
	}

	void onUpdate(AppWMs::Timer time) override{
		//if (AppWMs::Input::IsKeyPressed(258)) {
		//	APP_WMS_CLI_TRACE("TAB KEY")
		//}
		if (AppWMs::Input::IsKeyPressed( WMS_KEY_LEFT)) {
			pm_camera_pos.x -= pm_camera_pos_speed * time;
		}else if (AppWMs::Input::IsKeyPressed(WMS_KEY_RIGHT)) {
			pm_camera_pos.x += pm_camera_pos_speed * time;
		}

		if (AppWMs::Input::IsKeyPressed(WMS_KEY_UP)) {
			pm_camera_pos.y += pm_camera_pos_speed * time;
		}else if (AppWMs::Input::IsKeyPressed(WMS_KEY_DOWN)) {
			pm_camera_pos.y -= pm_camera_pos_speed * time;
		}


		if (AppWMs::Input::IsKeyPressed(WMS_KEY_A)) {
			pm_camera_rot += pm_camera_rot_speed * time;
		}
		else if (AppWMs::Input::IsKeyPressed(WMS_KEY_D)) {
			pm_camera_rot -= pm_camera_rot_speed * time;
		}

		if (AppWMs::Input::IsKeyPressed(WMS_KEY_F)) {
			pm_sq_pos.x -= pm_sq_pos_speed * time;
		}
		else if (AppWMs::Input::IsKeyPressed(WMS_KEY_H)) {
			pm_sq_pos.x += pm_sq_pos_speed * time;
		}

		if (AppWMs::Input::IsKeyPressed(WMS_KEY_T)) {
			pm_sq_pos.y += pm_sq_pos_speed * time;
		}
		else if (AppWMs::Input::IsKeyPressed(WMS_KEY_G)) {
			pm_sq_pos.y -= pm_sq_pos_speed * time;
		}


		// 
		// test for triangle
		AppWMs::RendererCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		AppWMs::RendererCommand::clear();
		// camera setup test
		pm_camera.setPosition(pm_camera_pos);
		pm_camera.setRotation(pm_camera_rot);

		//
		AppWMs::Renderer::beginScene(pm_camera);

		glm::vec4 red = { .8f, 0.2f, 0.3f, 1 };
		glm::vec4 blue = { 0.2f, 0.4f, 0.8f, 1 };
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		for (int i = 0; i < 5; i++) {
			glm::vec3 pos(i * 0.06f, 0.0f, 0.0f);
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos+ pm_sq_pos) * scale;
			if (i % 2 == 0) {
				pm_shader_ptr->uploadUniformFloat4("v_Color", red);
			}else{
				pm_shader_ptr->uploadUniformFloat4("v_Color", blue);
			}

			AppWMs::Renderer::submit(pm_vertexArray_ptr, pm_shader_ptr, transform);
		}
		AppWMs::Renderer::end();
	}

	void onEvent(AppWMs::Event& event) override{
		//if (event.getEventType() == AppWMs::EventType::KeyPressed) {
		//	AppWMs::KeyPressedEvent& e = (AppWMs::KeyPressedEvent&) event;
		//	APP_WMS_CLI_TRACE("{0}", (char)e.getKeyCode());
		//}
	}

	virtual void onImGuiRender() override {

	}

	//void onAttach() override {
	//	std::cout << "test" << std::endl;
	//}

private:
	AppWMs::Camera pm_camera;
	std::shared_ptr<AppWMs::Shader> pm_shader_ptr;
	std::shared_ptr<AppWMs::VertexArray> pm_vertexArray_ptr;
	glm::vec3 pm_camera_pos;
	float pm_camera_rot=0.0f;
	float pm_camera_pos_speed = 0.005f;
	float pm_camera_rot_speed = 1.0f;
	glm::vec3 pm_sq_pos = {0.0f, 0.0f, 0.0f};
	float pm_sq_pos_speed = 0.005f;
};



class Sandbox : public AppWMs::App
{
public:
	Sandbox() {
		pushLayer(new WindowLayer());
	}
	~Sandbox() {

	}

};

AppWMs::App* AppWMs::createApplication(){
	return new Sandbox();
}
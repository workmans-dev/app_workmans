#include "pch.h"
#include "renderer/renderer2D.h"
#include "openGL/openGLShader.h"
#include "renderer/rendererCommand.h"

namespace AppWMs {
	Scope<Renderer2D::Renderer_comp_data>Renderer2D::pm_renderer_comp_data =
		std::make_unique<Renderer2D::Renderer_comp_data>();

	ShaderLib Renderer2D::pm_ShLib;

	void Renderer2D::init()
	{
		// create vertexArray by using openGL-----
		pm_renderer_comp_data->vertexArray_ptr = VertexArray::create();

		float sq_verts[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 1.0f
		};

		// to add vertexBuffer to vertexArray-----
		Ref<VertexBuffer> pm_vertexBuffer_ptr;
		pm_vertexBuffer_ptr.reset(VertexBuffer::create(sq_verts, sizeof(sq_verts)));

		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float2, "a_TexCoord"}
		};
		pm_vertexBuffer_ptr->setLayout(layout);
		pm_renderer_comp_data->vertexArray_ptr->addVertexBuffer(pm_vertexBuffer_ptr);

		// to add indexBuffer to vertexArray-----
		unsigned int tri_indexes[3] = { 0,1,2 };
		unsigned int sq_indexes[6] = { 0,1,2,2,3,0 };
		Ref<IndexBuffer> pm_indexBuffer_ptr;
		pm_indexBuffer_ptr.reset(IndexBuffer::create(sq_indexes, sizeof(sq_indexes) / sizeof(uint32_t)));
		pm_renderer_comp_data->vertexArray_ptr->setIndexBuffer(pm_indexBuffer_ptr);

		// white !!!
		pm_renderer_comp_data->white_tex_ptr = Texture2D::create(1, 1);
		uint32_t whiteTex_data = 0xffffffff;
		pm_renderer_comp_data->white_tex_ptr->setData(&whiteTex_data, sizeof(uint32_t));

		// shader pointer define
		pm_renderer_comp_data->shader_ptr = pm_ShLib.load("assets/shaders/texture.glsl");
		pm_renderer_comp_data->shader_ptr->bind();
		pm_renderer_comp_data->shader_ptr->uploadUniformInt("u_Texture", 0);

	}



	void Renderer2D::beginScene(const CameraMatrix& camMat, const glm::mat4& transform)
	{
		APP_WMS_PROFILE_FUNC();
		const glm::mat4& view_proj_mat = camMat.get_projection()* glm::inverse(transform);
		// rendering on camera
		pm_renderer_comp_data->shader_ptr->bind();
		pm_renderer_comp_data->shader_ptr->uploadUniformMat4("u_view_proj", view_proj_mat);
	}

	void Renderer2D::endScene()

	{
	}

	void Renderer2D::drawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		APP_WMS_PROFILE_FUNC();
		// removed texture(icon) by white color, like an alphaC! 
		pm_renderer_comp_data->white_tex_ptr->bind();
		// set color
		pm_renderer_comp_data->shader_ptr->uploadUniformFloat4("u_Color", color);
		// scale and position  ------
		pm_renderer_comp_data->shader_ptr->uploadUniformMat4("u_transform", transform);

		// create shape
		pm_renderer_comp_data->vertexArray_ptr->bind();
		RendererCommand::drawIndexed(pm_renderer_comp_data->vertexArray_ptr);
		pm_renderer_comp_data->vertexArray_ptr->unbind();
	}
	void Renderer2D::drawQuad_texture(const std::string& fileName, const glm::mat4& transform,
		const Ref<Texture2D>& tex_ptr, const glm::vec4& tintColor,float tiling)
	{
		APP_WMS_PROFILE_FUNC();
		// get big_sq_shader_ptr---- stored by load function based on the fileName!! API
		auto shader_ptr = pm_ShLib.getShader(fileName);

		// removed color by white color, like an alphaC! 
		shader_ptr -> uploadUniformFloat4("u_Color", tintColor);
		// repeat texture as the pattern
		shader_ptr->uploadUniformFloat("u_Tile", tiling);

		tex_ptr->bind();
		shader_ptr->uploadUniformMat4("u_transform", transform);
		// create shape
		pm_renderer_comp_data->vertexArray_ptr->bind();
		RendererCommand::drawIndexed(pm_renderer_comp_data->vertexArray_ptr);

	}




}

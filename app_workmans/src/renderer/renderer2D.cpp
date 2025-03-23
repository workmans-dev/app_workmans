#include "pch.h"
#include "renderer/renderer2D.h"
#include "openGL/openGLShader.h"
#include "renderer/rendererCommand.h"
#include "renderer/uniformBuffer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


namespace AppWMs {
	struct QuadVertex {
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 texCoord;
		float texIndex;
		float pattern;
		int entityID;
	};

	struct Renderer_comp_data {
		static const uint32_t maxQuads = 10000;
		// each vertex will have a QuadVertex info.
		static const uint32_t maxVertexes = maxQuads * 4;
		static const uint32_t maxIndexes = maxQuads * 6;
		static const uint32_t maxTexSlots = 32;
		uint32_t quadIndexCount = 0;
		uint32_t texSlotIndex = 1;
		std::array<Ref<Texture2D>, maxTexSlots> texSlots_array;


		Ref<VertexArray> vertexArray_ptr;
		Ref<Shader> shader_ptr;
		Ref<Texture2D> white_tex_ptr;

		Ref<VertexBuffer> quadVBuffer_ptr;
		QuadVertex* quadVertexBase = nullptr;
		QuadVertex* quadVertexPtr = nullptr;

		Renderer2D::Statistics stats;

		struct CameraData
		{
			glm::mat4 view_proj_mat=glm::mat4(1.0f);
		};
		CameraData cameraData;
		Ref<UniformBuffer> cam_UB;
		Renderer2D::QuadInfo quadInfo;
		
	};




	static Renderer_comp_data render_dataSet;
	ShaderLib Renderer2D::pm_ShLib;


	void Renderer2D::init()
	{
		// create OpenGLVectexArray
		render_dataSet.vertexArray_ptr = VertexArray::create();
		std::cout << "created vertexArray" << std::endl;

		// create OpenGLVertexBuffer
		render_dataSet.quadVBuffer_ptr =
			VertexBuffer::create(render_dataSet.maxVertexes*sizeof(QuadVertex));

		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_Pattern"},
			{ShaderDataType::Int, "a_EntityID"}
		};
		render_dataSet.quadVBuffer_ptr->setLayout(layout);
		render_dataSet.vertexArray_ptr->addVertexBuffer(render_dataSet.quadVBuffer_ptr);

		render_dataSet.quadVertexBase = new QuadVertex[render_dataSet.maxVertexes];

		uint32_t* quadIndices = new uint32_t[render_dataSet.maxIndexes];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < render_dataSet.maxIndexes; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;
			offset += 4;
		}

		//std::unique_ptr<uint32_t[]> quadIndices = std::make_unique<uint32_t[]>(render_dataSet.maxIndexes);
		Ref<IndexBuffer> indexBuffer_ptr =IndexBuffer::create(quadIndices, render_dataSet.maxIndexes);
		render_dataSet.vertexArray_ptr->setIndexBuffer(indexBuffer_ptr);
		delete[] quadIndices;

		// Texture2D !!! for texture alpha
		render_dataSet.white_tex_ptr = Texture2D::create(1, 1);
		uint32_t whiteTex_data = 0xffffffff;
		render_dataSet.white_tex_ptr->setData(&whiteTex_data, sizeof(uint32_t));

		//// shader pointer define
		//int32_t samplers[render_dataSet.maxTexSlots];
		//for (uint32_t i = 0; i < render_dataSet.maxTexSlots; i++) {
		//	samplers[i] = i;
		//}
		render_dataSet.shader_ptr = pm_ShLib.load("assets/shaders/texture.glsl");
		//render_dataSet.shader_ptr->bind();
		//render_dataSet.shader_ptr->uploadUniformIntArray("u_Textures", samplers, render_dataSet.maxTexSlots);
		// set init array
		render_dataSet.texSlots_array[0] = render_dataSet.white_tex_ptr;
		render_dataSet.cam_UB = 
			UniformBuffer::create(sizeof(Renderer_comp_data::CameraData), 0);
	}



	void Renderer2D::beginScene(const CameraController& cam, const glm::mat4& transform)
	{
		APP_WMS_PROFILE_FUNC();
		render_dataSet.cameraData.view_proj_mat = cam.get_projection() * glm::inverse(transform);
		render_dataSet.cam_UB->setData(&render_dataSet.cameraData, sizeof(Renderer_comp_data::CameraData));
		//// rendering on camera
		
		//render_dataSet.shader_ptr->uploadUniformMat4("u_view_proj", render_dataSet.cameraData.view_proj_mat);

		render_dataSet.quadIndexCount = 0;
		render_dataSet.quadVertexPtr = render_dataSet.quadVertexBase;
		render_dataSet.texSlotIndex = 1;
	}

	void Renderer2D::endScene()
	{
		APP_WMS_PROFILE_FUNC();
		uint32_t size = (uint32_t)((uint8_t*)render_dataSet.quadVertexPtr - 
			(uint8_t*)render_dataSet.quadVertexBase);
		render_dataSet.quadVBuffer_ptr->setData(render_dataSet.quadVertexBase, size);

		flush();
	}

	void Renderer2D::flush()
	{
		for (uint32_t i = 0; i < render_dataSet.texSlotIndex; i++) {

			render_dataSet.texSlots_array[i]->bind(i);
		}

		render_dataSet.shader_ptr->bind();
		RendererCommand::drawIndexed(render_dataSet.vertexArray_ptr, render_dataSet.quadIndexCount);
		render_dataSet.stats.drawCalls++;
	}

	void Renderer2D::shutdown()
	{
		delete[] render_dataSet.quadVertexBase;
	}

	void Renderer2D::drawQuad(const glm::mat4& transform, const glm::vec4& color, int e_ID)
	{

		if (render_dataSet.quadIndexCount >= Renderer_comp_data::maxIndexes) {
			end_flush();
		}
		APP_WMS_PROFILE_FUNC();
		// VertexArray
		for (int n = 0; n < render_dataSet.quadInfo.vertexCount; n++) {
			render_dataSet.quadVertexPtr->position = transform * render_dataSet.quadInfo.vertPos[n];
			render_dataSet.quadVertexPtr->color = color;
			render_dataSet.quadVertexPtr->texCoord = render_dataSet.quadInfo.texCoords[n];
			render_dataSet.quadVertexPtr->texIndex = 0.0f;
			render_dataSet.quadVertexPtr->pattern = 1.0f;
			render_dataSet.quadVertexPtr->entityID = e_ID;
			render_dataSet.quadVertexPtr++;
		}
		render_dataSet.quadIndexCount += 6; // 2 x triangle = quad.
		render_dataSet.stats.quadCount++;

	}


	void Renderer2D::drawQuad(const glm::mat4& transform, const glm::vec4& tintColor,
		const std::string& fileName, const Ref<Texture2D>& tex_ptr, int e_ID, float tiling)
	{
		APP_WMS_PROFILE_FUNC();
		if (render_dataSet.quadIndexCount >= Renderer_comp_data::maxIndexes) {
			end_flush();
		}

		//constexpr glm::vec4 color = { 1.0f, 1.0f , 1.0f , 1.0f };
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i<render_dataSet.texSlotIndex; i++) {
			if (*render_dataSet.texSlots_array[i].get() == *tex_ptr.get()) {
				textureIndex = (float)i;
				std::cout << "index :" << i << "pointer" << tex_ptr.get() << std::endl;
				break;
			}
		}
		
		if (textureIndex==0.0f) {
			textureIndex = (float)render_dataSet.texSlotIndex;

			render_dataSet.texSlots_array[render_dataSet.texSlotIndex] = tex_ptr;
			render_dataSet.texSlotIndex++;
		}
		// VertexArray
		for (int n = 0; n < render_dataSet.quadInfo.vertexCount; n++) {
			render_dataSet.quadVertexPtr->position = transform * render_dataSet.quadInfo.vertPos[n];
			render_dataSet.quadVertexPtr->color = tintColor;
			render_dataSet.quadVertexPtr->texCoord = render_dataSet.quadInfo.texCoords[n];
			render_dataSet.quadVertexPtr->texIndex = textureIndex;
			render_dataSet.quadVertexPtr->pattern = 1.0f;
			render_dataSet.quadVertexPtr->entityID = e_ID;
			render_dataSet.quadVertexPtr++;
		}

		render_dataSet.quadIndexCount += 6; // 2 x triangle = quad.


		//tex_ptr->bind();
		//render_dataSet.shader_ptr->uploadUniformMat4("u_transform", transform);

		render_dataSet.stats.quadCount++;

	}

	Renderer2D::Statistics& Renderer2D::getStats()
	{
		return render_dataSet.stats;
	}




	void Renderer2D::resetStats() {
		memset(&render_dataSet.stats, 0, sizeof(Statistics));
	}

	void Renderer2D::end_flush()
	{
		endScene();
		render_dataSet.quadIndexCount = 0;
		render_dataSet.quadVertexPtr = render_dataSet.quadVertexBase;
		render_dataSet.texSlotIndex = 1;

	}

}

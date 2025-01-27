#include "pch.h"
#include "core.h"

#include "openGL/openGLVertexArray.h"
#include <glad/glad.h>


namespace AppWMs {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
		switch (type) {
		case AppWMs::ShaderDataType::Float: return GL_FLOAT;
		case AppWMs::ShaderDataType::Float2: return GL_FLOAT;
		case AppWMs::ShaderDataType::Float3: return GL_FLOAT;
		case AppWMs::ShaderDataType::Float4: return GL_FLOAT;
		case AppWMs::ShaderDataType::Mat3: return GL_FLOAT;
		case AppWMs::ShaderDataType::Mat4: return GL_FLOAT;
		case AppWMs::ShaderDataType::Int: return GL_INT;
		case AppWMs::ShaderDataType::Int2: return GL_INT;
		case AppWMs::ShaderDataType::Int3: return GL_INT;
		case AppWMs::ShaderDataType::Int4: return GL_INT;
		case AppWMs::ShaderDataType::Bool: return GL_BOOL;
		}

		APP_WMS_CORE_ASSERTS(false, "No compitable with GL_<>! !")
		return 0;
	}


	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &pm_renderID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &pm_renderID);
	}

	void OpenGLVertexArray::bind() const
	{
		glBindVertexArray(pm_renderID);
	}

	void OpenGLVertexArray::unbind() const
	{
		glBindVertexArray(0);
	}
	void OpenGLVertexArray::addVertexBuffer(const Ref<VertexBuffer> vertexBuffer)
	{
		glBindVertexArray(pm_renderID);
		vertexBuffer->bind();
		uint32_t index = 0;
		for (const BufferElements& e : vertexBuffer->getLayout()) {
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, e.getComponentCount(),
				ShaderDataTypeToOpenGLBaseType(e.shaderDataType),
				e.normalized ? GL_TRUE : GL_FALSE,
				vertexBuffer->getLayout().getStride(), (const void*)e.offset);
			index++;
		}
		pm_vertexBuffers.push_back(vertexBuffer);
	}
	void OpenGLVertexArray::setIndexBuffer(const Ref<IndexBuffer> indexBuffer)
	{
		glBindVertexArray(pm_renderID);
		indexBuffer->bind();
		pm_indexBuffer = indexBuffer;
	}
}
#include "pch.h"

#include "openGL/openGLVertexBuffer.h"
#include <glad/glad.h>


namespace AppWMs {
	//
	// OpenGLVertexBuffer
	//
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertexes, uint32_t size)
	{
		APP_WMS_PROFILE_FUNC();
		glCreateBuffers(1, &pm_renderID);
		glBindBuffer(GL_ARRAY_BUFFER, pm_renderID);
		glBufferData(GL_ARRAY_BUFFER, size, vertexes, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		APP_WMS_PROFILE_FUNC();
		glCreateBuffers(1, &pm_renderID);
		glBindBuffer(GL_ARRAY_BUFFER, pm_renderID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		APP_WMS_PROFILE_FUNC();
		glDeleteBuffers(1, &pm_renderID);
	}

	void OpenGLVertexBuffer::bind() const
	{
		APP_WMS_PROFILE_FUNC();
		glBindBuffer(GL_ARRAY_BUFFER, pm_renderID);
	}

	void OpenGLVertexBuffer::unbind() const
	{
		APP_WMS_PROFILE_FUNC();
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}

	void OpenGLVertexBuffer::setData(const void* VB, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, pm_renderID);
		// update portion without memory reallocating it which it has been already allocated before.
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, VB);
		//glNamedBufferSubData(m_RendererID, offset, size, data);
	}
	

	//
	// OpenGLIndexBuffer
	//
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indexes, uint32_t count):pm_count(count)
	{
		APP_WMS_PROFILE_FUNC();
		glCreateBuffers(1, &pm_renderID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pm_renderID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*sizeof(uint32_t), indexes, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		APP_WMS_PROFILE_FUNC();
		glDeleteBuffers(1, &pm_renderID);
	}

	void OpenGLIndexBuffer::bind() const
	{
		APP_WMS_PROFILE_FUNC();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pm_renderID);
	}

	void OpenGLIndexBuffer::unbind() const
	{
		APP_WMS_PROFILE_FUNC();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

}
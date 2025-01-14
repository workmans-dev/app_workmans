#include "pch.h"

#include "openGL/openGLVertexBuffer.h"
#include <glad/glad.h>


namespace AppWMs {
	//
	// OpenGLVertexBuffer
	//
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertexes, uint32_t size)
	{
		glCreateBuffers(1, &pm_renderID);
		glBindBuffer(GL_ARRAY_BUFFER, pm_renderID);
		glBufferData(GL_ARRAY_BUFFER, size, vertexes, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &pm_renderID);
	}

	void OpenGLVertexBuffer::bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, pm_renderID);
	}

	void OpenGLVertexBuffer::unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}
	
	//
	// OpenGLIndexBuffer
	//
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indexes, uint32_t count):pm_count(count)
	{
		glCreateBuffers(1, &pm_renderID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pm_renderID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*sizeof(uint32_t), indexes, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &pm_renderID);
	}

	void OpenGLIndexBuffer::bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pm_renderID);
	}

	void OpenGLIndexBuffer::unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

}
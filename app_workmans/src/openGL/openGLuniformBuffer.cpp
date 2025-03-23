#include "pch.h"

#include "openGL/openGLuniformBuffer.h"
#include <glad/glad.h>


namespace AppWMs {
	OpenGLuniformBuffer::OpenGLuniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &pm_renderID);
		glNamedBufferData(pm_renderID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, pm_renderID);
	}
	OpenGLuniformBuffer::~OpenGLuniformBuffer()
	{
		glDeleteBuffers(1, &pm_renderID);
	}


	void OpenGLuniformBuffer::setData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(pm_renderID, offset, size, data);
	}
}
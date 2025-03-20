#include "pch.h"

#include "openGL/openGLframeBuffer.h"
#include <glad/glad.h>
#include "core/core.h"

namespace AppWMs {
	OpenGLframeBuffer::OpenGLframeBuffer(const FrameBufferSpec& spec):pm_spec(spec)
	{
		invalidate();
	}


	OpenGLframeBuffer::~OpenGLframeBuffer()
	{
		glDeleteFramebuffers(1, &pm_renderID);
		glDeleteTextures(1, &pm_colorAttach);
		glDeleteTextures(1, &pm_depthAttach);
	}

	void OpenGLframeBuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pm_renderID);
		glViewport(0, 0, pm_spec.w, pm_spec.h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}

	void OpenGLframeBuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void OpenGLframeBuffer::resize(uint32_t w, uint32_t h)
	{
		pm_spec.w = w;
		pm_spec.h = h;
		invalidate();
	}


	void OpenGLframeBuffer::invalidate()
	{
		if (pm_renderID) {
			glDeleteFramebuffers(1, &pm_renderID);
			glDeleteTextures(1, &pm_colorAttach);
			glDeleteTextures(1, &pm_depthAttach);
		}
		// create buffer
		glCreateFramebuffers(1, &pm_renderID);
		glBindFramebuffer(GL_FRAMEBUFFER, pm_renderID);

		// get pm_colorAttach
		glCreateTextures(GL_TEXTURE_2D, 1, &pm_colorAttach);
		glTextureStorage2D(pm_colorAttach, 1, GL_RGBA8, pm_spec.w, pm_spec.h);
		glTextureParameteri(pm_colorAttach, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(pm_colorAttach, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glNamedFramebufferTexture(pm_renderID, GL_COLOR_ATTACHMENT0, pm_colorAttach, 0);


		glCreateTextures(GL_TEXTURE_2D, 1, &pm_depthAttach);
		glTextureStorage2D(pm_depthAttach, 1, GL_DEPTH24_STENCIL8, pm_spec.w, pm_spec.h);
		glNamedFramebufferTexture(pm_renderID, GL_DEPTH_STENCIL_ATTACHMENT, pm_depthAttach, 0);

		// Check Framebuffer Status
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
		}

		// unbind <--- important!
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

}
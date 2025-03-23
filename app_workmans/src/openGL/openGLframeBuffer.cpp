#include "pch.h"

#include "openGL/openGLframeBuffer.h"
#include <glad/glad.h>
#include "core/core.h"

namespace AppWMs {
	static const uint32_t s_maxFramebuffer_size = 8192;
	static bool isDepthFormat(FramebufferTextureFormat& format) {
		switch (format) {
		case FramebufferTextureFormat::DEPTHSTENCIL8: return true;

		}
		return false;
	}

	static GLenum TextureTarget(bool multiSampled) {
		return multiSampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	static void createTexture(bool multiSampled, uint32_t* outID, uint32_t count) {
		//	glCreateTextures(GL_TEXTURE_2D, 1, &pm_colorAttach);
		glCreateTextures(TextureTarget(multiSampled), count, outID);

	}
	static void bindTexture(bool multiSampled, uint32_t id) {
		glBindTexture(TextureTarget(multiSampled), id);
	}


	static void attachColorTexture(const uint32_t& id,const uint32_t& renderID, const uint32_t& samples, const GLenum& format,
		uint32_t& w, uint32_t& h, const int& index) {
		if (samples > 1) {
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format,
				w, h, GL_FALSE);
		}
		else {
			int mipLevels = static_cast<int>(std::floor(std::log2(std::max(w, h)))) + 1;
			glTextureStorage2D(id, mipLevels, format, w, h);
			glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		//(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
		// layer is not including yet, // level is 0
		glNamedFramebufferTexture(renderID, GL_COLOR_ATTACHMENT0+index, id, 0);

	}

	static void attachDepthTexture(const uint32_t& id, const uint32_t& renderID, const uint32_t& samples, const GLenum& format_1,
		const GLenum& format_2, uint32_t& w, uint32_t& h) {
		if (samples > 1) {
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format_1,
				w, h, GL_FALSE);
		}
		else {
			glTextureStorage2D(id, 1, format_1, w, h);

		}
		// layer is not including yet, // level is 0
		glNamedFramebufferTexture(renderID, format_2, id, 0);
	}

	OpenGLframeBuffer::OpenGLframeBuffer(const FrameBufferSpec& spec):pm_bufferSpec(spec)
	{
		for (FrameBufferTextureSpec& textureSpec : pm_bufferSpec.attachSpec.attachments) {
			if (!isDepthFormat(textureSpec.textureFormat)) {
				pm_textureSpec_vec.emplace_back(textureSpec);

			}
			else {
				pm_depthSpec = textureSpec;
			}
		}
		invalidate();
	}


	OpenGLframeBuffer::~OpenGLframeBuffer()
	{
		glDeleteFramebuffers(1, &pm_renderID);
		glDeleteTextures((GLsizei)pm_textureID_vec.size(), pm_textureID_vec.data());
		glDeleteTextures(1, &pm_depthID);

	}

	void OpenGLframeBuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pm_renderID);
		glViewport(0, 0, pm_bufferSpec.w, pm_bufferSpec.h);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}

	void OpenGLframeBuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void OpenGLframeBuffer::resize(uint32_t w, uint32_t h)
	{
		pm_bufferSpec.w = w;
		pm_bufferSpec.h = h;
		invalidate();
	}


	void OpenGLframeBuffer::invalidate()
	{
		if (pm_renderID) {
			glDeleteFramebuffers(1, &pm_renderID);
			glDeleteTextures((GLsizei)pm_textureID_vec.size(), pm_textureID_vec.data());
			glDeleteTextures(1, &pm_depthID);
			pm_textureID_vec.clear();
			pm_depthID = 0;
		}
		// create buffer
		glCreateFramebuffers(1, &pm_renderID);
		glBindFramebuffer(GL_FRAMEBUFFER, pm_renderID);

		bool multiSampled = pm_bufferSpec.samples > 1;
		if (pm_textureSpec_vec.size()) {
			pm_textureID_vec.resize(pm_textureSpec_vec.size());
			createTexture(multiSampled, pm_textureID_vec.data(), (uint32_t)pm_textureID_vec.size());
			for (int i = 0; i < pm_textureID_vec.size(); i++) {
				bindTexture(multiSampled, pm_textureID_vec[i]);
				switch (pm_textureSpec_vec[i].textureFormat) {
				case FramebufferTextureFormat::RGBA8:
					attachColorTexture(pm_textureID_vec[i], pm_renderID, pm_bufferSpec.samples, GL_RGBA8,
						pm_bufferSpec.w, pm_bufferSpec.h, i);
					break;
				}
			}
		}
		if (pm_depthSpec.textureFormat != FramebufferTextureFormat::None) {
			createTexture(multiSampled, &pm_depthID, 1);
			bindTexture(multiSampled, pm_depthID);
			switch (pm_depthSpec.textureFormat) {
			case FramebufferTextureFormat::DEPTHSTENCIL8:
				attachDepthTexture(pm_depthID, pm_renderID, pm_bufferSpec.samples, GL_DEPTH24_STENCIL8,
					GL_DEPTH_STENCIL_ATTACHMENT, pm_bufferSpec.w, pm_bufferSpec.h);
				break;
			}
		}

		if ((uint32_t)pm_textureID_vec.size() <= 4) {
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((GLsizei)pm_textureID_vec.size(), buffers);
		}
		else if (pm_textureID_vec.empty()) {
			glDrawBuffer(GL_NONE);
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "Framebuffer is not complete! Error: " << status << std::endl;
		}
		// unbind <--- important!
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

}
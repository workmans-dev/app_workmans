#include "pch.h"

#include "openGL/openGLTexture2D.h"
#include <glad/glad.h>

namespace AppWMs {
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path):pm_path(path)
	{
		int w, h, channel;
		stbi_uc* data = stbi_load(path.c_str(), &w, &h, &channel, 0);
		pm_height = h;
		pm_width = w;
		glCreateTextures(GL_TEXTURE_2D, 1, &pm_renderID);
		glTextureStorage2D(pm_renderID, GL_RGB8, pm_width, pm_height);

		glTextureParameteri(pm_renderID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(pm_renderID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureSubImage2D(pm_renderID, 0,0,0, pm_width, pm_height,
			GL_RGB, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &pm_renderID);
	}

	void OpenGLTexture2D::bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, pm_renderID);
	}

}
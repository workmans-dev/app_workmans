#include "pch.h"
#include "stb_image.h"
#include "openGL/openGLTexture2D.h"

namespace AppWMs {
	OpenGLTexture2D::OpenGLTexture2D(uint32_t w, uint32_t h):pm_width(w), pm_height(h)
	{
		pm_internal_format = GL_RGBA8, pm_data_format = GL_RGBA;
		glCreateTextures(GL_TEXTURE_2D, 1, &pm_renderID);
		glTextureStorage2D(pm_renderID, 1, pm_internal_format, w, h);

		glTextureParameteri(pm_renderID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(pm_renderID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path):pm_path(path)
	{
		APP_WMS_PROFILE_FUNC();
		int w, h, channel;
		stbi_uc* data = nullptr; 

		{
			SCOPE_PROFILE_TIMER("load texture");
			data = stbi_load(path.c_str(), &w, &h, &channel, 0);
		}

		APP_WMS_CORE_ASSERTS(data, "Failed to load image")
		pm_height = h;
		pm_width = w;

		pm_internal_format =0, pm_data_format = 0;
		if (channel == 4) {
			pm_internal_format = GL_RGBA8; pm_data_format = GL_RGBA;
		}
		else if (channel == 3) {
			pm_internal_format = GL_RGB8; pm_data_format = GL_RGB;
		}

		// get pm_renderID value
		glCreateTextures(GL_TEXTURE_2D, 1, &pm_renderID);
		glTextureStorage2D(pm_renderID, 1, pm_internal_format, pm_width, pm_height);

		glTextureParameteri(pm_renderID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(pm_renderID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureSubImage2D(pm_renderID, 0,0,0, pm_width, pm_height,
			pm_data_format, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &pm_renderID);
	}

	void OpenGLTexture2D::setData(void* data, uint32_t size)
	{
		glTextureSubImage2D(pm_renderID, 0, 0, 0, pm_width, pm_height,
			pm_data_format, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, pm_renderID);
	}

}
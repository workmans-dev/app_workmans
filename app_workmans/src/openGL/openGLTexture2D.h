#pragma once
#include <string>
#include "renderer/texture.h"
#include <glad/glad.h>


namespace AppWMs {

	class OpenGLTexture2D : public Texture2D {
	public:
		OpenGLTexture2D(uint32_t w, uint32_t h);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();
		virtual void setData(void* data, uint32_t size) override;
		virtual uint32_t get_renderID() const override {
			return pm_renderID;
		}
		virtual uint32_t getWidth() const override {
			return pm_width;
		}
		virtual uint32_t getHeight() const override {
			return pm_height;
		}
		virtual void bind(uint32_t slot=0) const override;
		virtual bool operator==(const Texture& other) const override {
			return pm_renderID == ((OpenGLTexture2D&)other).pm_renderID;
		}
	private:
		uint32_t pm_width, pm_height, pm_renderID;
		std::string pm_path;

		GLenum pm_internal_format=0;
		GLenum pm_data_format=0;
		

	};
}
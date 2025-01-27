#pragma once
#include <string>
#include "renderer/texture.h"


namespace AppWMs {
	class OpenGLTexture2D : public Texture2D {
	public:
		OpenGLTexture2D(const std::string& path);
		~OpenGLTexture2D();
		virtual uint32_t getWidth() const override {
			return pm_width;
		}
		virtual uint32_t getHeight() const override {
			return pm_height;
		}
		virtual void bind(uint32_t slot=0) const override;
	private:
		uint32_t pm_width, pm_height, pm_renderID;
		std::string pm_path;
		

	};
}
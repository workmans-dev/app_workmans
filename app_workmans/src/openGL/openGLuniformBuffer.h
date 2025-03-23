#pragma once

#include "renderer/uniformBuffer.h"

namespace AppWMs {
	class OpenGLuniformBuffer :public UniformBuffer {
	public:
		OpenGLuniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLuniformBuffer();
		virtual void setData(const void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		uint32_t pm_renderID; 

	};
}
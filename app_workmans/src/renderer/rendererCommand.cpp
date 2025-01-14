#include "pch.h"
#include "rendererCommand.h"

#include "openGL/openGLRenderer.h"

namespace AppWMs {

	RendererAPI* RendererCommand::pm_rendererAPI_ptr =
		new OpenGLRendererAPI;

}
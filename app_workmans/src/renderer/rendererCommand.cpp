#include "pch.h"
#include "rendererCommand.h"

#include "openGL/openGLRenderer.h"

namespace AppWMs {

	Scope<RendererAPI> RendererCommand::pm_rendererAPI_ptr =
		std::make_unique< OpenGLRendererAPI>();


}
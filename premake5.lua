workspace "app_workmans"
	architecture "x64"
	configurations{"Debug", "Release", "Dist"}

outputdir ="%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- statc lib 
includeDir = {}
includeDir["GLFW"] = "app_workmans/vendor/GLFW/include"
includeDir["Glad"] = "app_workmans/vendor/Glad/include"
includeDir["ImGui"] = "app_workmans/vendor/imgui"


include "app_workmans/vendor/GLFW"
include "app_workmans/vendor/Glad"
include "app_workmans/vendor/imgui"


project "app_workmans"
	location "app_workmans"
	kind "SharedLib"
	language "C++"
	targetdir ("bin/"..outputdir.. "/%{prj.name}")
	objdir ("bin-int/"..outputdir.. "/%{prj.name}")
	

	pchheader "pch.h"
	pchsource "%{prj.name}/src/pch.cpp"

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	includedirs{
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/src",
		"%{includeDir.GLFW}",
		"%{includeDir.Glad}",
		"%{includeDir.ImGui}"


	}
	-- static lib link
	links{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	filter "system:windows"
	cppdialect "C++20"
	systemversion "latest"
	staticruntime "off"

	defines{
		"APP_WMS_PLATFORM_WINDOWS",
		"APP_WMS_BUILD_DLL",
		"GLFW_INCLUDE_NONE",
	}
	postbuildcommands{
		"{MKDIR} ../bin/" .. outputdir .. "/sandbox", 
		"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/sandbox"
	}
	filter "configurations:Debug"
		defines "APP_WMS_DEBUG"
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines "APP_WMS_RELEASE"
		optimize "On"
		runtime "Release"

	filter "configurations:Dist"
		defines "APP_WMS_DIST"
		optimize "On"
		runtime "Debug"



project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"
	targetdir ("bin/"..outputdir.. "/%{prj.name}")
	objdir ("bin-int/"..outputdir.. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	includedirs{
		"app_workmans/vendor/spdlog/include",
		"app_workmans/src"
	}
	links{
		"app_workmans"
	}

	filter "system:windows"
	cppdialect "C++20"
	staticruntime "off"
	systemversion "latest"

	defines{
		"APP_WMS_PLATFORM_WINDOWS",
	}

	filter "configurations:Debug"
		defines "APP_WMS_DEBUG"
		symbols "On"
		runtime "Debug"

		
	filter "configurations:Release"
		defines "APP_WMS_RELEASE"
		optimize "On"
		runtime "Release"
		
	filter "configurations:Dist"
		defines "APP_WMS_DIST"
		optimize "On"
		runtime "Release"
		
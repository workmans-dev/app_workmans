workspace "app_workmans"
	architecture "x64"
	configurations{"Debug", "Release", "Dist"}

outputdir ="%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- statc lib 
includeDir = {}
includeDir["GLFW"] = "app_workmans/vendor/GLFW/include"
includeDir["Glad"] = "app_workmans/vendor/Glad/include"
includeDir["ImGui"] = "app_workmans/vendor/imgui"
includeDir["glm"] = "app_workmans/vendor/glm"
includeDir["stb_image"] = "app_workmans/vendor/stb_image"
includeDir["entt"] = "app_workmans/vendor/entt/include"
includeDir["yaml_cpp"] = "app_workmans/vendor/yaml-cpp/include"


group "Dependencies"
	include "app_workmans/vendor/GLFW"
	include "app_workmans/vendor/Glad"
	include "app_workmans/vendor/imgui"
	include "app_workmans/vendor/yaml-cpp"
group ""

project "app_workmans"
	location "app_workmans"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("bin/"..outputdir.. "/%{prj.name}")
	objdir ("bin-int/"..outputdir.. "/%{prj.name}")
	
	pchheader "pch.h"
	pchsource "%{prj.name}/src/pch.cpp"

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp"
	}
	defines{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"YAML_CPP_STATIC_DEFINE"
	}
	includedirs{
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/src",
		"%{includeDir.GLFW}",
		"%{includeDir.Glad}",
		"%{includeDir.ImGui}",
		"%{includeDir.glm}",
		"%{includeDir.stb_image}",
		"%{includeDir.entt}",
		"%{includeDir.yaml_cpp}"
	}

	-- static lib link
	links{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"yaml-cpp"
	}

	filter "system:windows"
		systemversion "latest"
		defines{
			"APP_WMS_PLATFORM_WINDOWS",
			"APP_WMS_BUILD_DLL"
		}
		buildoptions { "/utf-8" } 


	filter "configurations:Debug"
		defines "APP_WMS_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "APP_WMS_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "APP_WMS_DIST"
		optimize "on"
		runtime "Debug"



project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("bin/"..outputdir.. "/%{prj.name}")
	objdir ("bin-int/"..outputdir.. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	includedirs{
		"app_workmans/vendor/spdlog/include",
		"app_workmans/src",
		"app_workmans/vendor",
		"%{includeDir.glm}",
		"%{includeDir.entt}"
	}
	links{
		"app_workmans"
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" } 
		defines{
			"APP_WMS_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "APP_WMS_DEBUG"
		symbols "on"
		runtime "Debug"

		
	filter "configurations:Release"
		defines "APP_WMS_RELEASE"
		optimize "on"
		runtime "Release"
		
	filter "configurations:Dist"
		defines "APP_WMS_DIST"
		optimize "on"
		runtime "Release"
		
workspace "app_workmans"
    architecture "x64"
    startproject "sandbox"
    configurations{"Debug", "Release", "Dist"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Vulkan SDK 환경 변수 경로 가져오기
vulkan_sdk = os.getenv("VULKAN_SDK")

-- static lib 
includeDir = {}
includeDir["GLFW"] = "app_workmans/vendor/GLFW/include"
includeDir["ImGui"] = "app_workmans/vendor/imgui"
includeDir["glm"] = "app_workmans/vendor/glm"
includeDir["stb_image"] = "app_workmans/vendor/stb_image"
includeDir["entt"] = "app_workmans/vendor/entt/include"
includeDir["yaml_cpp"] = "app_workmans/vendor/yaml-cpp/include"
includeDir["ImGuizmo"] = "app_workmans/vendor/ImGuizmo"
includeDir["Vulkan"] = vulkan_sdk .. "/Include" -- Vulkan 헤더 경로 추가

group "Dependencies"
    include "app_workmans/vendor/GLFW"
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

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl",
        "%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp",
        "%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
        "%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE",
        "YAML_CPP_STATIC_DEFINE"
    }

    includedirs
    {
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/src",
        "%{includeDir.GLFW}",
        "%{includeDir.ImGui}",
        "%{includeDir.glm}",
        "%{includeDir.stb_image}",
        "%{includeDir.entt}",
        "%{includeDir.yaml_cpp}",
        "%{includeDir.ImGuizmo}",
        "%{includeDir.Vulkan}" -- Vulkan 인클루드 디렉토리 주입
    }

    -- 라이브러리 파일(.lib)을 찾을 경로 추가 (Vulkan)
    libdirs
    {
        vulkan_sdk .. "/Lib"
    }

    -- static lib link
    links{
        "GLFW",
        "ImGui",
        "vulkan-1.lib", -- opengl32.lib를 빼고 벌칸 라이브러리 추가
        "yaml-cpp"
    }

    filter "files:app_workmans/vendor/ImGuizmo/**.cpp"
    flags {"NoPCH"}

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
        "%{prj.name}/src/**.cpp",
    }
    includedirs{
        "app_workmans/vendor/spdlog/include",
        "app_workmans/src",
        "app_workmans/vendor",
        "%{includeDir.glm}",
        "%{includeDir.entt}",
        "%{includeDir.Vulkan}" -- 샌드박스도 벌칸 헤더 알 수 있도록 추가
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
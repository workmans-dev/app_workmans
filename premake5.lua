workspace "app_workmans"
    architecture "x64"
    startproject "sandbox"
    configurations{"Debug", "Release", "Dist"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Get the Vulkan SDK path from the environment variable
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
includeDir["VulkanMemoryAllocator"] = "app_workmans/vendor/VulkanMemoryAllocator"
includeDir["Vulkan"] = vulkan_sdk .. "/Include" -- Vulkan header path (includes the shaderc/ headers)

-- lib path / file names
libraryDir = {}
libraryDir["Vulkan"] = vulkan_sdk .. "/Lib"

library = {}
library["Vulkan"]  = "%{libraryDir.Vulkan}/vulkan-1.lib"
library["ShaderC"] = "%{libraryDir.Vulkan}/shaderc_combined.lib"

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
    removefiles
    {
        "%{prj.name}/src/renderer/shader.cpp",
        "%{prj.name}/src/renderer/texture.cpp",
        "%{prj.name}/src/renderer/renderer2D.cpp",
        "%{prj.name}/src/renderer/uniformBuffer.cpp",
        -- frameBuffer.cpp: the Vulkan factory wiring was finished in 2-c -> back in the build
        --
        -- vk_shaderCompile.cpp: the runtime GLSL compilation (shaderc) path.
        -- The SDK's shaderc_combined.lib ships only as /MD (dynamic release CRT), which
        -- collides with our /MTd setting at link time. Shaders are baked by
        -- compile_shaders.bat at build time, and the runtime only reads .spv (vk_shaderModule.cpp).
        -- If hot reload becomes necessary, build shaderc from source as /MTd and revive this.
        "%{prj.name}/src/vulkan/vk_shaderCompile.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE",
        "YAML_CPP_STATIC_DEFINE",
        -- Vulkan NDC depth is 0..1 (OpenGL is -1..1). Without this, glm::perspective builds a
        -- -1..1 matrix and half the depth gets clipped.
        -- It has to go in via the compiler /D so it is defined before the glm headers for certain.
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
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
        "%{includeDir.VulkanMemoryAllocator}",
        "%{includeDir.Vulkan}"
    }

    -- Add the path where library files (.lib) are found (Vulkan)
    libdirs
    {
        "%{libraryDir.Vulkan}"
    }

    -- static lib link
    links{
        "GLFW",
        "ImGui",
        "yaml-cpp",
        "%{library.Vulkan}"
        -- ShaderC unlinked -- CRT mismatch (/MD vs /MTd). See the removefiles comment above.
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
        runtime "Release" 


project "sandbox"
    location "sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    targetdir ("bin/"..outputdir.. "/%{prj.name}")
    objdir ("bin-int/"..outputdir.. "/%{prj.name}")

    -- sandbox uses the PCH too. app_workmans/src is in includedirs, so
    --    "pch.h" resolves to app_workmans/src/pch.h (the header is shared).
    --    The .pch binary has to be baked per project, though, so only
    --    pchsource is sandbox's own.
    pchheader "pch.h"
    pchsource "%{prj.name}/src/pch.cpp"

    files{
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }
    -- Has to match the engine. If only one side defines it, the same glm type compiles
    -- with a different depth convention per translation unit.
    defines{
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
    }
    includedirs{
        "app_workmans/vendor/spdlog/include",
        "app_workmans/src",
        "app_workmans/vendor",
        "%{includeDir.glm}",
        "%{includeDir.entt}",
        "%{includeDir.VulkanMemoryAllocator}",
        "%{includeDir.Vulkan}" -- added so sandbox knows the Vulkan headers too
    }

    links{
        "app_workmans"
    }

    -- Bakes the shaders to SPIR-V on every build. The runtime only reads .spv, so
    -- editing a shader and building makes it take effect right away.
    prebuildcommands {
        'call "$(SolutionDir)compile_shaders.bat"'
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
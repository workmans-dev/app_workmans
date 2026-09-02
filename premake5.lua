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
includeDir["Vulkan"] = vulkan_sdk .. "/Include" -- Vulkan 헤더 경로 추가 (shaderc/ 헤더 포함)

-- lib 경로 / 파일 이름
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
        -- frameBuffer.cpp: 2-c에서 벌칸 팩토리 배선 완료 → 빌드 복귀
        --
        -- vk_shaderCompile.cpp: 런타임 GLSL 컴파일(shaderc) 경로.
        -- SDK의 shaderc_combined.lib이 /MD(동적 릴리스 CRT)로만 배포되어
        -- 우리 /MTd 설정과 링크 충돌한다. 셰이더는 compile_shaders.bat이
        -- 빌드 시점에 굽고, 런타임은 .spv만 읽는다(vk_shaderModule.cpp).
        -- 핫 리로드가 필요해지면 shaderc를 /MTd로 소스 빌드해서 되살릴 것.
        "%{prj.name}/src/vulkan/vk_shaderCompile.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE",
        "YAML_CPP_STATIC_DEFINE",
        -- 벌칸 NDC 깊이는 0~1 (OpenGL은 -1~1). 이게 없으면 glm::perspective가
        -- -1~1 행렬을 만들어 깊이 절반이 클리핑된다.
        -- 컴파일러 /D로 넣어야 glm 헤더보다 확실히 먼저 정의된다.
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
        "%{includeDir.Vulkan}"
    }

    -- 라이브러리 파일(.lib)을 찾을 경로 추가 (Vulkan)
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
        -- ShaderC 링크 해제 — CRT 불일치(/MD vs /MTd). 위 removefiles 주석 참고.
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

    -- 🟢 sandbox도 PCH를 쓴다. includedirs에 app_workmans/src가 있어서
    --    "pch.h"는 app_workmans/src/pch.h로 해석된다(헤더는 공유).
    --    다만 .pch 바이너리는 프로젝트마다 따로 구워야 하므로
    --    pchsource만 sandbox 자기 것을 가진다.
    pchheader "pch.h"
    pchsource "%{prj.name}/src/pch.cpp"

    files{
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }
    -- 엔진과 동일하게 맞춰야 한다. 한쪽만 정의되면 같은 glm 타입이
    -- 번역 단위마다 다른 깊이 규약으로 컴파일된다.
    defines{
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
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

    -- 셰이더를 빌드마다 SPIR-V로 굽는다. 런타임은 .spv만 읽으므로
    -- 셰이더를 고친 뒤 빌드하면 바로 반영된다.
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
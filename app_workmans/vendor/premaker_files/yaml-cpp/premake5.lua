project "yaml-cpp"
    kind "StaticLib"
    language "C++"          -- [중요] C가 아니라 C++로 변경해야 합니다.
    staticruntime "on"
    warnings "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- yaml-cpp 소스 파일 및 헤더 파일 경로 지정
    files
    {
        "include/**.h",     -- include 폴더 내의 모든 헤더 파일 (재귀적)
        "src/**.cpp",       -- src 폴더 내의 모든 cpp 파일 (재귀적)
        "src/**.h"          -- src 폴더 내의 내부 헤더 파일
    }
	includedirs
	{
		"include"
	}

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"
		
	filter "system:linux"
		pic "on"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		optimize "on"
		runtime "Release"
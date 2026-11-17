project "yaml-cpp"
    kind "StaticLib"
    language "C++"          -- [important] this has to be C++, not C.
    staticruntime "on"
    warnings "off"
    defines { "YAML_CPP_STATIC_DEFINE" }
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- Source and header file paths for yaml-cpp
    files
    {
        "include/**.h",     -- every header in the include folder (recursive)
        "src/**.cpp",       -- every cpp in the src folder (recursive)
        "src/**.h"          -- the internal headers in the src folder
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
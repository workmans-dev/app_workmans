@echo off
REM ---------------------------------------------------------------
REM  GLSL -> SPIR-V precompile
REM
REM  Runtime shaderc linking was dropped (shaderc_combined.lib ships
REM  /MD only, clashes with our /MTd). Shaders are baked at build time
REM  instead; the runtime only reads .spv files.
REM
REM  premake wires this into sandbox prebuildcommands, so editing a
REM  shader and rebuilding is enough - no manual step.
REM
REM  Standalone use: compile_shaders.bat
REM
REM  NOTE: keep this file ASCII-only. cmd.exe reads it in the system
REM  codepage (949 here), so UTF-8 text breaks parsing.
REM ---------------------------------------------------------------
setlocal enabledelayedexpansion

if "%VULKAN_SDK%"=="" (
    echo [shaders] ERROR: VULKAN_SDK is not set.
    exit /b 1
)

set "GLSLC=%VULKAN_SDK%\Bin\glslc.exe"
if not exist "%GLSLC%" (
    echo [shaders] ERROR: glslc.exe not found - %GLSLC%
    exit /b 1
)

set "SHADER_DIR=%~dp0sandbox\assets\shaders"
set COUNT=0
set ERRORS=0

REM NOTE: do not use "exit /b" inside the for block - the code does not
REM reliably propagate to the caller. Count failures and exit at the end.
for %%f in ("%SHADER_DIR%\*.vert" "%SHADER_DIR%\*.frag" "%SHADER_DIR%\*.comp" "%SHADER_DIR%\*.geom") do (
    echo [shaders] %%~nxf
    "%GLSLC%" "%%f" -o "%%f.spv"
    if errorlevel 1 (
        echo [shaders] FAILED: %%~nxf
        set /a ERRORS+=1
    ) else (
        set /a COUNT+=1
    )
)

if !ERRORS! GTR 0 (
    echo [shaders] %ERRORS% file^(s^) failed to compile
    endlocal & exit /b 1
)

echo [shaders] compiled !COUNT! file^(s^)
endlocal & exit /b 0

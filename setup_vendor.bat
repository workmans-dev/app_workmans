@echo off
REM ---------------------------------------------------------------
REM  Vendor setup - run once after cloning, before gen_project_win.bat
REM
REM  1. Copies the premake files into the submodules. They are not part
REM     of the upstream repositories, so they cannot be committed there.
REM     The originals live in app_workmans/vendor/premaker_files.
REM
REM  2. Registers those copies, the generated project files and the
REM     build output in each submodule's .git/info/exclude.
REM
REM     info/exclude is local to the clone and is not tracked, so no
REM     upstream file is touched and "git pull" inside a submodule never
REM     conflicts. Appending the same rules to a submodule's .gitignore
REM     would do the opposite - .gitignore IS tracked upstream, so the
REM     submodule would show as modified and pulls would fight it.
REM
REM  NOTE: keep this file ASCII-only. cmd.exe reads it in the system
REM  codepage, so non-ASCII text breaks parsing.
REM ---------------------------------------------------------------
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
set "VENDOR=%ROOT%app_workmans\vendor"
set "TEMPLATES=%VENDOR%\premaker_files"
set "MODULES=%ROOT%.git\modules\app_workmans\vendor"

if not exist "%VENDOR%\imgui\imgui.h" (
    echo [setup] ERROR: submodules are not checked out.
    echo [setup] Run: git submodule update --init --recursive
    exit /b 1
)

echo [setup] copying premake files
for %%d in (GLFW imgui yaml-cpp) do (
    if not exist "%TEMPLATES%\%%d\premake5.lua" (
        echo [setup] ERROR: missing template "%TEMPLATES%\%%d\premake5.lua"
        exit /b 1
    )
    copy /y "%TEMPLATES%\%%d\premake5.lua" "%VENDOR%\%%d\premake5.lua" >nul
    if errorlevel 1 (
        echo [setup] ERROR: failed to copy premake5.lua into vendor\%%d
        exit /b 1
    )
    echo [setup]   premake5.lua -^> vendor\%%d
)

echo [setup] registering local excludes
for %%d in (GLFW ImGuizmo glm imgui spdlog yaml-cpp) do (
    set "GITDIR=%MODULES%\%%d"
    if exist "!GITDIR!\config" (
        if not exist "!GITDIR!\info" mkdir "!GITDIR!\info"
        findstr /c:"app_workmans setup" "!GITDIR!\info\exclude" >nul 2>&1
        if errorlevel 1 (
            >>"!GITDIR!\info\exclude" echo.
            >>"!GITDIR!\info\exclude" echo # --- app_workmans setup ^(local only, not tracked^) ---
            >>"!GITDIR!\info\exclude" echo premake5.lua
            >>"!GITDIR!\info\exclude" echo bin/
            >>"!GITDIR!\info\exclude" echo bin-int/
            >>"!GITDIR!\info\exclude" echo *.vcxproj
            >>"!GITDIR!\info\exclude" echo *.vcxproj.filters
            >>"!GITDIR!\info\exclude" echo *.vcxproj.user
            >>"!GITDIR!\info\exclude" echo *.sln
            echo [setup]   excludes -^> %%d
        ) else (
            echo [setup]   excludes already present -^> %%d
        )
    ) else (
        echo [setup]   skipped %%d ^(no git dir at "!GITDIR!"^)
    )
)

echo [setup] done. Next: gen_project_win.bat
endlocal & exit /b 0

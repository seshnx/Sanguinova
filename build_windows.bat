@echo off
setlocal enabledelayedexpansion
REM Build script for Sanguinova VST3 on Windows
REM This script configures and builds the plugin for Windows

echo ========================================
echo Sanguinova Build Script
echo Blood Star Distortion
echo ========================================
echo.
echo Current directory: %CD%
echo.

REM Change to script directory
cd /d "%~dp0"
if errorlevel 1 (
    echo ERROR: Failed to change to script directory!
    echo Script location: %~dp0
    pause
    exit /b 1
)
echo Changed to directory: %CD%
echo.

REM Try to find CMake
set "CMAKE_PATH="
if exist "C:\Program Files\CMake\bin\cmake.exe" (
    set "CMAKE_PATH=C:\Program Files\CMake\bin\cmake.exe"
    echo Found CMake in Program Files
    goto :cmake_found
)
if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
    set "CMAKE_PATH=C:\Program Files (x86)\CMake\bin\cmake.exe"
    echo Found CMake in Program Files (x86)
    goto :cmake_found
)

REM CMake not found
echo.
echo ERROR: CMake not found in standard locations.
echo.
echo Searched locations:
echo   C:\Program Files\CMake\bin\cmake.exe
echo   C:\Program Files (x86)\CMake\bin\cmake.exe
echo.
echo Please install CMake from https://cmake.org/download/
echo Or add CMake to your PATH and run this script again.
echo.
pause
exit /b 1

:cmake_found
echo CMake path set successfully
echo.
REM Test if CMake actually works
if not defined CMAKE_PATH (
    echo ERROR: CMAKE_PATH variable not set!
    pause
    exit /b 1
)

call "%CMAKE_PATH%" --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake found but cannot execute!
    echo Path was: "%CMAKE_PATH%"
    echo.
    pause
    exit /b 1
)
echo CMake version check passed.
echo.

REM Find Visual Studio (check newest first)
set "VS_PATH="
set "VS_VERSION="
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_VERSION=18"
    echo Found Visual Studio 18 Community
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_VERSION=18"
    echo Found Visual Studio 18 Professional
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_VERSION=2022"
    echo Found Visual Studio 2022 Community
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_VERSION=2022"
    echo Found Visual Studio 2022 Professional
) else if exist "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_VERSION=2019"
    echo Found Visual Studio 2019 Community
)

if "%VS_PATH%"=="" (
    echo ERROR: Visual Studio not found!
    echo Please install Visual Studio 2019 or 2022 with C++ tools.
    pause
    exit /b 1
) else (
    echo Using Visual Studio environment
    set USE_VS_ENV=1
)
echo.

REM Store source directory path
set "SOURCE_DIR=%~dp0"
set "SOURCE_DIR=%SOURCE_DIR:~0,-1%"
if not defined SOURCE_DIR (
    echo ERROR: Could not determine source directory
    pause
    exit /b 1
)

REM Create build directory
echo Creating build directory...
if not exist build mkdir build

REM Check if clean build requested
if "%1"=="clean" (
    echo Cleaning old build files...
    if exist build\CMakeCache.txt del /q build\CMakeCache.txt
    if exist build\CMakeFiles rmdir /s /q build\CMakeFiles
    if exist build\_deps rmdir /s /q build\_deps
    echo Clean complete.
)
echo.

REM Configure CMake
echo ========================================
echo Configuring CMake...
echo ========================================
REM Set up Visual Studio environment
echo Setting up Visual Studio environment...
call "%VS_PATH%"
if errorlevel 1 (
    echo ERROR: Failed to set up VS environment!
    pause
    exit /b 1
)
echo Visual Studio environment configured.
echo.

REM Determine Visual Studio generator based on detected version
set "VS_GENERATOR="
set "VS_ARCH=x64"

if "%VS_VERSION%"=="18" (
    set "VS_GENERATOR=Visual Studio 18 2026"
    echo Using generator: Visual Studio 18 2026
) else if "%VS_VERSION%"=="2022" (
    set "VS_GENERATOR=Visual Studio 17 2022"
    echo Using generator: Visual Studio 17 2022
) else if "%VS_VERSION%"=="2019" (
    set "VS_GENERATOR=Visual Studio 16 2019"
    echo Using generator: Visual Studio 16 2019
) else (
    echo ERROR: Could not determine Visual Studio generator
    pause
    exit /b 1
)
echo.

REM Run CMake configuration
echo Running CMake configuration...
echo This may take a while on first run (downloading JUCE)...
echo.
"%CMAKE_PATH%" -S "%SOURCE_DIR%" -B "%SOURCE_DIR%\build" -G "%VS_GENERATOR%" -A %VS_ARCH%

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: CMake configuration failed!
    echo ========================================
    echo.
    echo Please check:
    echo   1. Visual Studio is installed with C++ tools
    echo   2. CMake can find the Visual Studio installation
    echo   3. You have write permissions in the build directory
    echo   4. Internet connection for downloading JUCE
    echo.
    pause
    exit /b 1
)

echo CMake configuration successful!
echo.

echo.
echo ========================================
echo Building VST3 plugin (Release)...
echo ========================================
call "%VS_PATH%" >nul 2>&1
"%CMAKE_PATH%" --build "%SOURCE_DIR%\build" --config Release --target Sanguinova_VST3

if errorlevel 1 (
    echo.
    echo ERROR: VST3 Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Building Standalone (Release)...
echo ========================================
"%CMAKE_PATH%" --build "%SOURCE_DIR%\build" --config Release --target Sanguinova_Standalone

if errorlevel 1 (
    echo.
    echo WARNING: Standalone build failed, but VST3 should be available.
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Output files:
echo.

set "VST3_PATH=%SOURCE_DIR%\build\Sanguinova_artefacts\Release\VST3\Sanguinova.vst3"
set "STANDALONE_PATH=%SOURCE_DIR%\build\Sanguinova_artefacts\Release\Standalone\Sanguinova.exe"

if exist "%VST3_PATH%" (
    echo   VST3: %VST3_PATH%
) else (
    echo   VST3: NOT FOUND
)

if exist "%STANDALONE_PATH%" (
    echo   Standalone: %STANDALONE_PATH%
) else (
    echo   Standalone: NOT FOUND
)

echo.
echo To install VST3, copy to: C:\Program Files\Common Files\VST3\
echo.
pause

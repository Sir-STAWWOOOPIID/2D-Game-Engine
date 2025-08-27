@echo off
echo Setting up 2D Game Engine build environment...
echo.

REM Check for Visual Studio installation
echo Checking for Visual Studio installation...
set VS_FOUND=0
set VS_VERSION=

REM Check for Visual Studio 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set VS_VERSION=Visual Studio 2022 Community
    goto :vs_found
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set VS_VERSION=Visual Studio 2022 Professional
    goto :vs_found
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set VS_VERSION=Visual Studio 2022 Enterprise
    goto :vs_found
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set VS_VERSION=Visual Studio 2019 Community
    goto :vs_found
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set VS_VERSION=Visual Studio 2019 Professional
    goto :vs_found
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set VS_VERSION=Visual Studio 2019 Enterprise
    goto :vs_found
)

REM If we get here, no Visual Studio was found
echo ERROR: Visual Studio not found!
echo.
echo This project requires Visual Studio with C++ build tools.
echo Please install one of the following:
echo   - Visual Studio 2022 Community (free): https://visualstudio.microsoft.com/vs/community/
echo   - Visual Studio 2019 Community (free): https://visualstudio.microsoft.com/vs/older-downloads/
echo   - Or any Professional/Enterprise version
echo.
echo During installation, make sure to select "Desktop development with C++"
echo.
pause
exit /b 1

:vs_found
echo Found: %VS_VERSION%
echo.

REM Check for Git
echo Checking for Git...
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Git not found in PATH!
    echo.
    echo Please install Git from: https://git-scm.com/download/win
    echo Make sure to add Git to your PATH during installation.
    echo.
    pause
    exit /b 1
)

echo Found Git: 
git --version
echo.

REM Check for CMake
echo Checking for CMake...
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found in PATH!
    echo.
    echo Please install CMake from: https://cmake.org/download/
    echo Make sure to add CMake to your PATH during installation.
    echo.
    pause
    exit /b 1
)

echo Found CMake:
cmake --version
echo.

REM Check if vcpkg already exists
if exist vcpkg (
    echo vcpkg already exists, updating...
    cd vcpkg
    git pull
    if %errorlevel% neq 0 (
        echo ERROR: Failed to update vcpkg!
        echo.
        echo Please delete the vcpkg folder and run setup.bat again.
        echo.
        pause
        exit /b 1
    )
    cd ..
) else (
    echo Installing vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    if %errorlevel% neq 0 (
        echo ERROR: Failed to clone vcpkg repository!
        echo.
        echo Please check your internet connection and try again.
        echo.
        pause
        exit /b 1
    )
    cd vcpkg
    call bootstrap-vcpkg.bat
    if %errorlevel% neq 0 (
        echo ERROR: Failed to bootstrap vcpkg!
        echo.
        echo Please check the error messages above and try again.
        echo.
        pause
        exit /b 1
    )
    call vcpkg integrate install
    if %errorlevel% neq 0 (
        echo WARNING: vcpkg integration failed. This may require administrator privileges.
        echo You can continue, but you may need to run as administrator later.
        echo.
    )
    cd ..
)

echo.
echo Checking installed dependencies...
cd vcpkg

REM Check if raylib is already installed
echo Checking raylib...
call vcpkg list | findstr "raylib" >nul
if %errorlevel% equ 0 (
    echo raylib is already installed, skipping...
    goto :raylib_skip
) else (
    echo raylib not found, will install...
    goto :raylib_install
)

:raylib_install
echo Installing raylib...
call vcpkg install raylib:x64-windows
if %errorlevel% neq 0 (
    echo ERROR: Failed to install raylib!
    echo.
    echo Please check the error messages above and try again.
    echo.
    pause
    exit /b 1
)
echo raylib installed successfully.
goto :python_check

:raylib_skip
echo raylib installation skipped (already present).

:python_check
REM Check if Python3 is already installed
echo Checking Python3...
call vcpkg list | findstr "python3" >nul
if %errorlevel% equ 0 (
    echo Python3 is already installed, skipping...
    goto :python_skip
) else (
    echo Python3 not found, will install...
    goto :python_install
)

:python_install
echo Installing Python3...
call vcpkg install python3:x64-windows
if %errorlevel% neq 0 (
    echo ERROR: Failed to install Python3!
    echo.
    echo Please check the error messages above and try again.
    echo.
    pause
    exit /b 1
)
echo Python3 installed successfully.
goto :finish

:python_skip
echo Python3 installation skipped (already present).

:finish
cd ..

echo.
echo ========================================
echo Setup completed successfully!
echo ========================================
echo.
echo Found: %VS_VERSION%
echo Dependencies: raylib and Python3 are ready
echo.
echo You can now run build.ps1 to build the project.
echo.
pause
exit /b 0

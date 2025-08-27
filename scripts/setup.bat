@echo off
echo Setting up 2D Game Engine build environment...
echo.
REM Check for Visual Studio installation
REM ...existing code...
REM Check for Git
REM ...existing code...
REM Check for CMake
REM ...existing code...
REM Check if vcpkg already exists
if exist ..\vcpkg (
    echo vcpkg already exists, updating...
    cd ..\vcpkg
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
    git clone https://github.com/Microsoft/vcpkg.git ..\vcpkg
    if %errorlevel% neq 0 (
        echo ERROR: Failed to clone vcpkg repository!
        echo.
        echo Please check your internet connection and try again.
        echo.
        pause
        exit /b 1
    )
    cd ..\vcpkg
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
echo Installing dependencies from vcpkg manifest...
cd ..\vcpkg
call vcpkg install
if %errorlevel% neq 0 (
    echo ERROR: Failed to install dependencies from vcpkg manifest!
    echo.
    echo Please check the error messages above and try again.
    echo.
    pause
    exit /b 1
)
cd ..

echo.
echo ========================================
echo Setup completed successfully!
echo ========================================
echo.
echo Found: %VS_VERSION%
echo Dependencies: raylib and Python3 are ready
echo.
echo You can now run scripts\build.ps1 to build the project.
echo.
pause
exit /b 0

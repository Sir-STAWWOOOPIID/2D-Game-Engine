Write-Host "Building 2D Game Engine..." -ForegroundColor Green

# Create build directory
if (!(Test-Path "build")) { New-Item -ItemType Directory -Name "build" }
Set-Location build

# Configure with CMake (using vcpkg)
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Release

Write-Host "Build complete! Executable is in build/bin/Release/" -ForegroundColor Green
Read-Host "Press Enter to continue"

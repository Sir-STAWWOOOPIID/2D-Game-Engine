Write-Host "Building 2D Game Engine..." -ForegroundColor Green

# Create build directory if it doesn't exist
if (!(Test-Path "../build")) { New-Item -ItemType Directory -Path "../build" }
Set-Location ../build

# Configure with CMake using vcpkg toolchain (from local vcpkg folder)
cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

# Build (Release configuration)
cmake --build . --config Release

Write-Host "Build complete! Executable is in build/bin/Release/" -ForegroundColor Green
Read-Host "Press Enter to continue"

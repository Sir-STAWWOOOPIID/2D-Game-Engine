Write-Host "Building 2D Game Engine..." -ForegroundColor Green

# Create build directory if it doesn't exist
if (!(Test-Path "../build")) { New-Item -ItemType Directory -Path "../build" }
Set-Location ../build

# Configure with CMake using vcpkg toolchain (from local vcpkg folder)
if (!(Test-Path "CMakeCache.txt")) {
    Write-Host "Configuring project with CMake..."
    cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
} else {
    Write-Host "CMake configuration already exists. Skipping configuration."
}

# Build (Release configuration)
Write-Host "Building project..."
cmake --build . --config Release

Write-Host "Build complete! Executable is in build/bin/Release/" -ForegroundColor Green
Read-Host "Press Enter to continue"

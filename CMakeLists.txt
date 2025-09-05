cmake_minimum_required(VERSION 3.16)
project(2d-game-engine C)

# Use vcpkg toolchain if available
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
add_executable(2d-game-engine src/main.c src/python_integration.c src/file_io.c src/engine_api.c)

target_include_directories(2d-game-engine PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find raylib (via vcpkg)
find_package(raylib CONFIG REQUIRED)
target_link_libraries(2d-game-engine PRIVATE raylib)

# Find Python (via vcpkg)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
target_link_libraries(2d-game-engine PRIVATE Python3::Python)

# Windows-specific settings
if(MSVC)
    target_compile_options(2d-game-engine PRIVATE /W4)
else()
    target_compile_options(2d-game-engine PRIVATE -Wall -Wextra)
endif()

# Set output directory
set_target_properties(2d-game-engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy Python DLL after build (optional, for Windows)
if(WIN32)
    add_custom_command(TARGET 2d-game-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Python3::Python>
        $<TARGET_FILE_DIR:2d-game-engine>
    )
endif()
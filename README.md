# 2D Game Engine with Python Integration

A powerful 2D game engine built with raylib and Python scripting support.

## Features

- **Visual Editor**: Drag-and-drop game object creation
- **Python Scripting**: Write game logic in Python
- **Custom Scripting Language**: Built-in scripting for simple operations
- **Real-time Preview**: See changes instantly
- **Project Management**: Save and load game projects
- **Syntax Highlighting**: Full Python and custom script support

## Quick Start Options

### Option 1: Simple MSYS2 Build (Recommended - Easiest)
**Prerequisites:** [MSYS2](https://www.msys2.org/) (includes GCC, raylib, Python)

```bash
# 1. Install MSYS2 from https://www.msys2.org/
# 2. Add C:\msys64\mingw64\bin to your PATH
# 3. Clone and build:
git clone https://github.com/yourusername/2D-Game-Engine.git
cd 2D-Game-Engine
build-simple.bat
```

### Option 2: vcpkg with MinGW (Advanced)
**Prerequisites:** MSYS2, [CMake](https://cmake.org/download/), [Git](https://git-scm.com/)

```bash
# 1. Setup vcpkg environment
setup-mingw.bat

# 2. Build the project
build-mingw.bat
```

### Option 3: vcpkg with Visual Studio (Windows Native)
**Prerequisites:** [Visual Studio](https://visualstudio.microsoft.com/), [CMake](https://cmake.org/download/), [Git](https://git-scm.com/)

```bash
# 1. Setup vcpkg environment
setup.bat

# 2. Build the project
build.bat
```

## Alternative Build Methods

### Method 1: Standalone Dependencies (Old Method)
If you prefer to use standalone installations:

```bash
# Install raylib and Python separately, then use:
make win RAYLIB_ROOT=C:/raylib/raylib PYROOT=C:/Python313
```

### Method 2: MSYS2 MinGW64
```bash
# Install MSYS2, then:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-raylib mingw-w64-x86_64-python
mingw32-make
```

## Usage

### Basic Commands
- `addCircle name x y size r g b` - Create a circle
- `addRect name x y size r g b` - Create a rectangle
- `move name dx dy` - Move an object
- `color name r g b` - Change object color
- `NewScript name` - Create a new script file
- `NewPython name` - Create a new Python script
- `RunScript name` - Execute a script
- `RunPython name.py` - Execute a Python script

### Python API
```python
# Move an object
move_object("player", 10, 0)

# Set position
set_position("enemy", 100, 200)

# Change color
set_color("player", 255, 0, 0)

# Create objects
create_circle("bullet", 50, 50, 5, 255, 255, 0)
create_rect("wall", 200, 100, 20, 128, 128, 128)

# Log messages
log_message("Game started!")
```

### Custom Scripting Language
```
# Variables
speed = 5.0
angle = 0.0

# Object manipulation
move player speed 0
setPos enemy 100 200
setColor player 255 0 0

# Math expressions
x = sin(angle) * 100
y = cos(angle) * 100
```

## Project Structure
```
2D-Game-Engine/
├── main.c              # Main source code
├── CMakeLists.txt      # CMake build configuration
├── build.bat           # Windows build script
├── build.ps1           # PowerShell build script
├── Makefile            # Alternative make-based build
├── Projects/           # Game projects directory
│   └── Default/
│       ├── main.game   # Project file
│       └── scripts/    # Script files
└── README.md           # This file
```

## Troubleshooting

### Common Issues

1. **CMake not found**: Install CMake from https://cmake.org/download/
2. **vcpkg integration failed**: Run `./vcpkg integrate install` as administrator
3. **Python DLL missing**: The CMake build automatically copies the Python DLL
4. **Build fails**: Ensure you have Visual Studio Build Tools installed

### Getting Help
- Check that all prerequisites are installed
- Ensure vcpkg dependencies are properly installed
- Try building in a clean directory
- Check the build output for specific error messages

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test the build process
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [raylib](https://www.raylib.com/) - Game development library
- [Python](https://www.python.org/) - Programming language
- [vcpkg](https://github.com/Microsoft/vcpkg) - C++ package manager

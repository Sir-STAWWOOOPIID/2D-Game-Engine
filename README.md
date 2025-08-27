# 2D Game Engine with Python Integration

A cross-platform 2D game engine built with [raylib](https://www.raylib.com/) and Python scripting support.

## Features
- Visual Editor: Drag-and-drop game object creation
- Python Scripting: Write game logic in Python
- Custom Scripting Language: Built-in scripting for simple operations
- Real-time Preview: See changes instantly
- Project Management: Save and load game projects
- Syntax Highlighting: Full Python and custom script support

## Project Structure
```
2D-Game-Engine/
├── src/                # Source files (main.c, ...)
├── include/            # Header files (if any)
├── scripts/            # Build and setup scripts (setup.bat, build.ps1, ...)
├── vcpkg/              # vcpkg (dependency manager)
├── build/              # Build output (ignored by git)
├── CMakeLists.txt      # CMake build configuration
├── vcpkg.json          # vcpkg manifest
├── .gitignore
├── README.md
└── Projects/           # Game projects directory
```

## Prerequisites
- **Windows:**
  - [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) (with Desktop development with C++)
  - [CMake](https://cmake.org/download/)
  - [Git](https://git-scm.com/download/win)
- **Other platforms:** Not officially supported in this repo structure, but raylib and Python are cross-platform.

## Setup & Build (Windows, Visual Studio)
1. **Clone the repository:**
   ```sh
   git clone https://github.com/Sir-STAWWOOOPIID/2D-Game-Engine.git
   cd 2D-Game-Engine
   ```
2. **Run setup script to install dependencies:**
   ```sh
   scripts\setup.bat
   ```
   - Installs vcpkg (if not present) and required libraries (raylib, python3).
3. **Build the project:**
   ```sh
   scripts\build.ps1
   ```
   - Uses CMake and MSVC to build. Output is in `build/bin/Release/`.

## Usage
- Launch the built executable from `build/bin/Release/`.
- Use the visual editor and scripting features as described below.

### Basic Commands
- `addCircle name x y size r g b` - Create a circle
- `addRect name x y size r g b` - Create a rectangle
- `move name dx dy` - Move an object
- `color name r g b` - Change object color
- `NewScript name` - Create a new script file
- `NewPython name` - Create a new Python script
- `RunScript name` - Execute a script
- `RunPython name.py` - Execute a Python script

### Python API Example
```python
move_object("player", 10, 0)
set_position("enemy", 100, 200)
set_color("player", 255, 0, 0)
create_circle("bullet", 50, 50, 5, 255, 255, 0)
create_rect("wall", 200, 100, 20, 128, 128, 128)
log_message("Game started!")
```

### Custom Scripting Example
```
speed = 5.0
angle = 0.0
move player speed 0
setPos enemy 100 200
setColor player 255 0 0
x = sin(angle) * 100
y = cos(angle) * 100
```

## Troubleshooting
- **CMake not found:** Install CMake from https://cmake.org/download/
- **vcpkg integration failed:** Run `scripts/setup.bat` as administrator
- **Python DLL missing:** The build script copies the Python DLL automatically
- **Build fails:** Ensure Visual Studio Build Tools are installed and vcpkg dependencies are present

## Contributing
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test the build process
5. Submit a pull request

## License
MIT License - see the LICENSE file for details.

## Acknowledgments
- [raylib](https://www.raylib.com/) - Game development library
- [Python](https://www.python.org/) - Programming language
- [vcpkg](https://github.com/Microsoft/vcpkg) - C++ package manager

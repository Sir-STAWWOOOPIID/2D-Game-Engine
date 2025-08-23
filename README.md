# 🎮 2D Game Engine v1.0.0

A lightweight and beginner-friendly **2D game engine** written in C with Python scripting support.  
This first release includes basic object creation, movement, and rendering—perfect for small projects and learning game development fundamentals.  

---

## 🚀 Download
- 👉 [**Get the latest release (v1.0.0)**](https://github.com/Sir-STAWWOOOPIID/2D-Game-Engine/releases/download/1.0.0/game.exe)
- ⚠️ Warning, this exe ***only*** works if you have python installed in your local disk C:

---

## 🧩 Features
- Simple **Python scripting API** for creating and controlling objects.
- Support for **rectangles** and **circles** with custom size and color.
- Functions to **move, position, recolor, and delete objects**.
- Built-in **logging system** for debugging.
- Minimal setup—just run the executable and start scripting!
- Creates a Projects folder in the same root
- You can even use a seperate file editor for the scripts

---

## 🗎 Info:
- Used **[Ralib](https://www.raylib.com)** for engine design.
- Used **[Python](https://www.python.org)** for an **API-like** python text editor.
- Download gcc **[Here](https://sourceforge.net/projects/mingw-w64)**.

---

## 📦 Compilation

This project was compiled using **GCC**. Example command:

```bash
gcc -IC:\raylib\raylib\src -IC:\Python313\include -LC:\raylib\raylib\src -LC:\Python313\libs -o 2Deditor.exe main.c -lraylib -lpython313 -lopengl32 -lgdi32 -lwinmm
```

## 📜 Python API Reference
Here are the main functions you can use in your game scripts:

```html
move_object(<name>, <x>, <y>)

set_position(<name>, <x>, <y>)

set_color(<name>, <r>, <g>, <b>)

create_circle(<name>, <x>, <y>, <size>, <r>, <g>, <b>)

create_rect(<name>, <x>, <y>, <size>, <r>, <g>, <b>)

delete_obj(<name>)

log_message(<message>)
```

---

## 💻 Active Commands

* `addCircle <name> <x> <y> <size> <r> <g> <b>`'

* `addRect <name> <x> <y> <size> <r> <g> <b>`
* `move <name> <x> <y>`
* `color <name> <r> <g> <b>`
* `update <project name>`
* `delete <name>`
* `set <variable name> <value>`
* `NewScript <name>`
* `NewPython <name>`
* `RunScript <script>`
* `RunPython <name>`

---

# 🎮 2D Game Engine v1.0.0

A lightweight and beginner-friendly **2D game engine** written in C with Python scripting support.  
This first release includes basic object creation, movement, and rendering—perfect for small projects and learning game development fundamentals.  

---

## 🚀 Download
👉 [**Get the latest release (v1.0.0)**](https://github.com/Sir-STAWWOOOPIID/2D-Game-Engine/releases/download/1.0.0/game.exe)

---

## 🧩 Features
- Simple **Python scripting API** for creating and controlling objects.
- Support for **rectangles** and **circles** with custom size and color.
- Functions to **move, position, recolor, and delete objects**.
- Built-in **logging system** for debugging.
- Minimal setup—just run the executable and start scripting!

---

## 📜 Python API Reference
Here are the main functions you can use in your game scripts:

```diff
move_object(<name>, <x>, <y>)

set_position(<name>, <x>, <y>)

set_color(<name>, <r>, <g>, <b>)

create_circle(<name>, <x>, <y>, <size>, <r>, <g>, <b>)

create_rect(<name>, <x>, <y>, <size>, <r>, <g>, <b>)

delete_obj(<name>)

log_message(<message>)

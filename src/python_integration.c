// Python integration moved to separate translation unit to avoid Windows headers colliding with raylib

// Prevent extra Windows headers where possible
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif

#include <Python.h>
#include "../include/python_integration.h"
#include "../include/ui.h"
#include <stdio.h>
#include <string.h>

// Externals from main.c
extern void AddLog(const char* fmt, ...);
extern void ExecuteCommand(const char* cmd);
extern const char currentProject[];

bool InitializePython(void) {
    Py_Initialize();
    if (!Py_IsInitialized()) {
        AddLog("Failed to initialize Python");
        return false;
    }

    PyRun_SimpleString(
        "# Game Engine API\n"
        "game_objects = {}\n"
        "game_variables = {}\n"
        "game_commands = []\n"
        "\n"
        "def move_object(name, dx, dy):\n"
        "    cmd = f'move {name} {dx} {dy}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Moving {name} by {dx}, {dy}')\n"
        "\n"
        "def set_position(name, x, y):\n"
        "    cmd = f'setPos {name} {x} {y}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Setting {name} position to {x}, {y}')\n"
        "\n"
        "def set_color(name, r, g, b):\n"
        "    cmd = f'color {name} {r} {g} {b}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Setting {name} color to {r}, {g}, {b}')\n"
        "\n"
        "def create_circle(name, x, y, size, r, g, b):\n"
        "    cmd = f'addCircle {name} {x} {y} {size} {r} {g} {b}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Creating circle {name}')\n"
        "\n"
        "def create_rect(name, x, y, size, r, g, b):\n"
        "    cmd = f'addRect {name} {x} {y} {size} {r} {g} {b}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Creating rectangle {name}')\n"
        "\n"
        "def delete_obj(name):\n"
        "    cmd = f'delete {name}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Deleting object: {name}')\n"
        "\n"
        "def log_message(msg):\n"
        "    print(f'[Python] {msg}')\n"
        "\n"
        "import math\n"
        "import time\n"
    );

    AddLog("Python initialized with game API");
    return true;
}

void ExecutePythonScript(const char* scriptName) {
    char scriptPath[256];
    snprintf(scriptPath, sizeof(scriptPath), "./Projects/%s/scripts/%s", currentProject, scriptName);

    FILE* testFile = fopen(scriptPath, "r");
    if(!testFile) {
        AddLog("Python script file '%s' not found at path: %s", scriptName, scriptPath);
        return;
    }
    fclose(testFile);

    PyRun_SimpleString("game_commands.clear()");

    FILE* file = fopen(scriptPath, "r");
    if(!file) {
        AddLog("Failed to open Python script '%s'", scriptName);
        return;
    }

    char pythonCode[8192] = {0};
    char line[256];
    while(fgets(line, sizeof(line), file)) {
        strncat(pythonCode, line, sizeof(pythonCode) - strlen(pythonCode) - 1);
    }
    fclose(file);

    if(PyRun_SimpleString(pythonCode) != 0) {
        AddLog("Python script execution failed for '%s'", scriptName);
        return;
    }

    PyObject* commandsList = PyObject_GetAttrString(PyImport_AddModule("__main__"), "game_commands");
    if(commandsList && PyList_Check(commandsList)) {
        Py_ssize_t size = PyList_Size(commandsList);
        for(Py_ssize_t i = 0; i < size; i++) {
            PyObject* item = PyList_GetItem(commandsList, i);
            if(PyUnicode_Check(item)) {
                const char* cmd = PyUnicode_AsUTF8(item);
                if(cmd) ExecuteCommand(cmd);
            }
        }
    }

    AddLog("Executed Python script '%s' from project '%s'", scriptName, currentProject);
}

void FinalizePython(void) {
    if(Py_IsInitialized()) Py_Finalize();
}

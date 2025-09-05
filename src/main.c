#define RAYLIB_NO_CONFLICT_WINDOWS
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <direct.h>
#include <math.h>
#include <ctype.h>

#include "../include/core.h"
#include "../include/visual_editor.h"
#include "../include/script_engine.h"
#include "../include/ui.h"
#include "../include/engine_io.h"
#include "../include/python_integration.h"

// Color constants
const Color ACCENT_RED = {230, 41, 55, 255};
const Color ACCENT_BLUE = {0, 121, 241, 255};

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 800, "2D Game Engine with Python Integration");
    SetTargetFPS(60);
    
    Font customFont = LoadFont("C:/Fonts/Codey/static/SourceCodePro-Bold.ttf");
    if (customFont.texture.id == 0) 
        customFont = LoadFont("./fonts/SourceCodePro-Bold.ttf");
    if (customFont.texture.id == 0) { 
        customFont = GetFontDefault(); 
        AddLog("Failed to load custom font, using default"); 
    } else { 
        AddLog("Custom font loaded successfully"); 
    }

    if(!InitializePython()) { 
        AddLog("Warning: Python initialization failed"); 
    }

    AddObject("player", SHAPE_CIRCLE, (Vector2){500,300}, 40, ACCENT_RED);
    AddObject("box", SHAPE_RECT, (Vector2){700,350}, 50, ACCENT_BLUE);

    CreateDirIfNotExist("./Projects/Default/scripts");
    LoadScripts("Default");

    SetVariable("speed", 5.0f);
    SetVariable("angle", 0.0f);

    while(!WindowShouldClose()) {
        UpdateUI();
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawUI();
        EndDrawing();
    }

    FinalizePython();
    CloseWindow();
    return 0;
}
// --- Platform Defines to Avoid Windows Header Pollution ---
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif

// --- Engine Components ---
#include "../include/core.h"
#include "../include/visual_editor.h"
#include "../include/script_engine.h"
#include "../include/ui.h"
#include "../include/engine_io.h"
#include "../include/python_integration.h"

// --- Raylib and Third-Party Includes (after platform defines) ---
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

char currentProject[NAME_LEN] = "Default";

// --- Local IO wrappers ---
static void CreateDirIfNotExist_local(const char* path){ CreateDirIfNotExist(path); }
static void UpdateProject_local(const char* name){
    // TODO: Move to ProjectManager if needed
}
static void LoadScripts_local(const char* projectName){
    LoadScripts(projectName);
}
static void SaveScript_local(ScriptEditor* ed){
    SaveScript(ed);
}

// --- Main ---
int main(void){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200,800,"2D Game Engine with Python Integration");
    SetTargetFPS(60);
    Font customFont = LoadFont("C:/Fonts/Codey/static/SourceCodePro-Bold.ttf");
    if (customFont.texture.id == 0) customFont = LoadFont("./fonts/SourceCodePro-Bold.ttf");
    if (customFont.texture.id == 0) { customFont = GetFontDefault(); AddLog("Failed to load custom font, using default"); } else { AddLog("Custom font loaded successfully"); }

    if(!InitializePython()) { AddLog("Warning: Python initialization failed"); }

    AddObject("player",SHAPE_CIRCLE,(Vector2){500,300},40,ACCENT_RED);
    AddObject("box",SHAPE_RECT,(Vector2){700,350},50,ACCENT_BLUE);

    CreateDirIfNotExist_local("./Projects/Default/scripts");
    LoadScripts_local(currentProject);

    SetVariable("speed", 5.0f);
    SetVariable("angle", 0.0f);

    Rectangle playButton={(float)GetScreenWidth()-120,10,100,40};
    int editorCharY = 18;

    bool running = false;

    while(!WindowShouldClose()){
        const int screenW = GetScreenWidth();
        const int screenH = GetScreenHeight();
        const int commandBarH=40; const int logBarH=80; const int tabBarH=35; const int leftPanelW=220; const int bottomH=commandBarH+logBarH;
        Vector2 mouse=GetMousePosition();

        // Example: migrate input handling and UI logic to UI module
        // For now, just call a stub function to avoid build errors
        // TODO: Replace with actual UI update/draw function
        // UI_UpdateAndDraw(screenW, screenH, mouse, customFont, playButton, editorCharY);
    }

    FinalizePython();
    CloseWindow();
    return 0;
}
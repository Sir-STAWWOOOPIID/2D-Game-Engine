#ifndef CORE_H
#define CORE_H

#define _CRT_SECURE_NO_WARNINGS
#define RAYLIB_NO_CONFLICT_WINDOWS
#include <raylib.h>

// Shared constants
#define NAME_LEN 32
#define MAX_OBJECTS 100
#define CMD_LEN 256
#define LOG_CAP 100
#define MAX_SCRIPTS 32
#define MAX_OPEN_EDITORS 8

// Shape type
typedef enum {
    SHAPE_CIRCLE,
    SHAPE_RECT
} ShapeType;

// Game object
typedef struct {
    char name[NAME_LEN];
    Vector2 position;
    float size;
    Color color;
    ShapeType shape;
    bool selected;
} GameObject;

// Core functionality
void AddLog(const char* fmt, ...);

#endif // CORE_H

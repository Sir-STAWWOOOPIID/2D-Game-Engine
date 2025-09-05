#pragma once

#include <stdbool.h>

#define NAME_LEN 32
#define LINE_LEN 128
#define MAX_SCRIPTS 64

// Simple script container used by file I/O module
typedef struct {
    char name[NAME_LEN];
    char lines[128][LINE_LEN];
    int lineCount;
    bool isScript;
    bool isPython;
} Script;

typedef struct {
    char name[NAME_LEN];
    char lines[128][LINE_LEN];
    int lineCount;
    int scroll;
    bool isScript;
    bool isPython;
} ScriptEditor;

// Lightweight object representation for project save (no raylib types)
typedef struct {
    char name[NAME_LEN];
    int shape; // 0=circle,1=rect
    float x;
    float y;
    float size;
    int r,g,b;
} ObjectIO;

// Logging hook - implemented in main.c
extern void AddLog(const char* fmt, ...);

// File I/O functions
void CreateDirIfNotExist(const char* path);
void UpdateProjectIO(const char* name, const ObjectIO* objects, int objectCount);
int LoadScriptsIO(const char* projectName, Script* outScripts, int maxScripts);
int SaveScriptToProject(const char* projectName, const char* filename, char lines[][LINE_LEN], int lineCount);

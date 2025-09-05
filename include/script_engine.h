#ifndef SCRIPT_ENGINE_H
#define SCRIPT_ENGINE_H

#include "core.h"
#include "engine_io.h"
#include <stdbool.h>

// Script editor state
typedef struct {
    char filename[NAME_LEN];
    char* content;
    int cursorPos;
    bool dirty;
} ScriptEditor;

// Global script buffer
extern char scripts[MAX_SCRIPTS][NAME_LEN];
extern int scriptCount;

// Script engine functionality
bool InitializePython(void);
void FinalizePython(void);
void SetVariable(const char* name, float value);
float GetVariable(const char* name);
void ExecuteScript(const char* filename);
void LoadScript(const char* filename, ScriptEditor* editor);
void SaveScript(ScriptEditor* editor);

#endif // SCRIPT_ENGINE_H

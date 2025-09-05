#ifndef UI_H
#define UI_H

#include "core.h"
#include "script_engine.h"

// Forward declarations
Color GetScriptTokenColor(const char* token);

// UI state
extern bool showConsole;
extern bool showEditor;
extern bool showLogWindow;

// Editor data
extern ScriptEditor openEditors[MAX_OPEN_EDITORS];
extern int editorCount;

// Command and log buffers
extern char commandBuffer[CMD_LEN];
extern char logs[LOG_CAP][CMD_LEN];
extern char scriptList[MAX_SCRIPTS][NAME_LEN];
extern int scriptCount;
extern int logCount;

// UI functionality
void DrawUI(void);
void UpdateUI(void);
void AddLog(const char* fmt, ...);

#endif // UI_H

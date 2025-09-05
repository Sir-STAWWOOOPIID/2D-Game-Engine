#ifndef UI_H
#define UI_H

#include "core.h"

// UI logic: tab management, input, command bar, logs, drawing
void ExecuteCommand(const char* cmd);
void OpenScriptTab(const char* name);
void AddLog(const char* fmt, ...);
float MeasureTextSubstring(const char *text, int n, int fontSize);
Color GetScriptTokenColor(const char* token);

// UI state
extern ScriptEditor openEditors[MAX_OPEN_EDITORS];
extern int openCount;
extern int activeEditor;
extern int caretLine, caretCol;
extern bool visualEditorOpen;
extern char commandBuffer[CMD_LEN];
extern int commandLen;
extern int logCount;
extern char logs[LOG_CAP][CMD_LEN];
extern char suggestions[MAX_SCRIPTS][NAME_LEN];
extern int suggestionCount;
extern int highlightedSuggestion;

#endif // UI_H

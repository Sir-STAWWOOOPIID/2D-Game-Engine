#ifndef SCRIPT_ENGINE_H
#define SCRIPT_ENGINE_H

#include "core.h"
#include "engine_io.h"
#include <stdbool.h>

// Expose scripts and scriptCount for other modules
extern Script scripts[MAX_SCRIPTS];
extern int scriptCount;

// Variable system
float GetVariable(const char* name);
void SetVariable(const char* name, float value);

// Script execution
void ExecuteScriptLine(const char* line);
void ExecuteScript(const char* scriptName);

// Expression evaluation
float EvaluateExpression(const char* expr);

// Python integration
#include "python_integration.h"

// Script loading/saving
void LoadScripts(const char* projectName);
void SaveScript(ScriptEditor* ed);

#endif // SCRIPT_ENGINE_H

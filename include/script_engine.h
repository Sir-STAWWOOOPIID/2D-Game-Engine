#ifndef SCRIPT_ENGINE_H
#define SCRIPT_ENGINE_H

#include "core.h"
#include <stdbool.h>

// Variable system
float GetVariable(const char* name);
void SetVariable(const char* name, float value);

// Script execution
void ExecuteScriptLine(const char* line);
void ExecuteScript(const char* scriptName);

// Expression evaluation
float EvaluateExpression(const char* expr);

// Python integration
bool InitializePython(void);
void ExecutePythonScript(const char* scriptName);
void FinalizePython(void);

// Script loading/saving
void LoadScripts(const char* projectName);
void SaveScript(ScriptEditor* ed);

#endif // SCRIPT_ENGINE_H

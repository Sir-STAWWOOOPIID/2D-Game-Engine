#include "../include/script_engine.h"
#include "../include/core.h"
#include "../include/ui.h"
#include "../include/visual_editor.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

static Variable variables[MAX_VARIABLES];
static int variableCount = 0;
static Script scripts[MAX_SCRIPTS];
static int scriptCount = 0;

float GetVariable(const char* name) {
    for(int i = 0; i < variableCount; i++) {
        if(strcmp(variables[i].name, name) == 0) return variables[i].value;
    }
    return 0.0f;
}

void SetVariable(const char* name, float value) {
    for(int i = 0; i < variableCount; i++) {
        if(strcmp(variables[i].name, name) == 0) { variables[i].value = value; return; }
    }
    if(variableCount < MAX_VARIABLES) {
        strncpy(variables[variableCount].name, name, NAME_LEN-1);
        variables[variableCount].value = value;
        variableCount++;
    }
}

float EvaluateExpression(const char* expr) {
    char cleanExpr[256] = {0};
    strncpy(cleanExpr, expr, 255);
    char* src = cleanExpr;
    char* dst = cleanExpr;
    while(*src) { if(*src != ' ') *dst++ = *src; src++; }
    *dst = '\0';

    if(isdigit(cleanExpr[0]) || cleanExpr[0] == '-') return atof(cleanExpr);
    char* plus = strchr(cleanExpr, '+');
    char* minus = strchr(cleanExpr, '-');
    char* mult = strchr(cleanExpr, '*');
    char* div = strchr(cleanExpr, '/');
    if(plus) { *plus = '\0'; return EvaluateExpression(cleanExpr) + EvaluateExpression(plus + 1); }
    if(minus && minus != cleanExpr) { *minus = '\0'; return EvaluateExpression(cleanExpr) - EvaluateExpression(minus + 1); }
    if(mult) { *mult = '\0'; return EvaluateExpression(cleanExpr) * EvaluateExpression(mult + 1); }
    if(div) { *div = '\0'; float denominator = EvaluateExpression(div + 1); return denominator != 0 ? EvaluateExpression(cleanExpr) / denominator : 0; }
    if(strncmp(cleanExpr, "sin(", 4) == 0) { char* end = strchr(cleanExpr, ')'); if(end) { *end='\0'; return sin(EvaluateExpression(cleanExpr+4) * PI / 180.0f); } }
    if(strncmp(cleanExpr, "cos(", 4) == 0) { char* end = strchr(cleanExpr, ')'); if(end) { *end='\0'; return cos(EvaluateExpression(cleanExpr+4) * PI / 180.0f); } }
    return GetVariable(cleanExpr);
}

void ExecuteScriptLine(const char* line) {
    if(!line || strlen(line)==0 || line[0]=='#') return;
    char cmd[CMD_LEN]; strncpy(cmd,line,CMD_LEN-1); cmd[CMD_LEN-1]='\0';
    char word[64]; if(sscanf(cmd, "%63s", word) != 1) return;
    char* equals = strchr(cmd,'=');
    if(equals){ *equals='\0'; char varName[64]; sscanf(cmd, "%63s", varName); float value = EvaluateExpression(equals+1); SetVariable(varName, value); AddLog("[Script] %s = %.2f", varName, value); return; }
    if(strcmp(word,"move")==0){ char objName[NAME_LEN], dxExpr[64], dyExpr[64]; if(sscanf(cmd,"move %31s %63s %63s", objName, dxExpr, dyExpr)==3){ GameObject* obj=FindByName(objName); if(obj){ float dx=EvaluateExpression(dxExpr); float dy=EvaluateExpression(dyExpr); obj->position = Vector2Add(obj->position, (Vector2){dx,dy}); } } return; }
    if(strcmp(word,"setPos")==0){ char objName[NAME_LEN], xExpr[64], yExpr[64]; if(sscanf(cmd,"setPos %31s %63s %63s", objName, xExpr, yExpr)==3){ GameObject* obj=FindByName(objName); if(obj){ float x=EvaluateExpression(xExpr); float y=EvaluateExpression(yExpr); obj->position=(Vector2){x,y}; } } return; }
    if(strcmp(word,"setColor")==0){ char objName[NAME_LEN], rExpr[64], gExpr[64], bExpr[64]; if(sscanf(cmd,"setColor %31s %63s %63s %63s", objName, rExpr, gExpr, bExpr)==4){ GameObject* obj=FindByName(objName); if(obj){ int r=(int)EvaluateExpression(rExpr); int g=(int)EvaluateExpression(gExpr); int b=(int)EvaluateExpression(bExpr); obj->color=(Color){r,g,b,255}; } } return; }
    if(strcmp(word,"createCircle")==0){ char objName[NAME_LEN], a0[64],a1[64],a2[64],a3[64],a4[64],a5[64]; if(sscanf(cmd,"createCircle %31s %63s %63s %63s %63s %63s %63s", objName,a0,a1,a2,a3,a4,a5)==7){ float x=EvaluateExpression(a0), y=EvaluateExpression(a1), size=EvaluateExpression(a2); int r=(int)EvaluateExpression(a3), g=(int)EvaluateExpression(a4), b=(int)EvaluateExpression(a5); AddObject(objName,SHAPE_CIRCLE,(Vector2){x,y},size,(Color){r,g,b,255}); } return; }
    if(strcmp(word,"createRect")==0){ char objName[NAME_LEN], a0[64],a1[64],a2[64],a3[64],a4[64],a5[64]; if(sscanf(cmd,"createRect %31s %63s %63s %63s %63s %63s %63s", objName,a0,a1,a2,a3,a4,a5)==7){ float x=EvaluateExpression(a0), y=EvaluateExpression(a1), size=EvaluateExpression(a2); int r=(int)EvaluateExpression(a3), g=(int)EvaluateExpression(a4), b=(int)EvaluateExpression(a5); AddObject(objName,SHAPE_RECT,(Vector2){x,y},size,(Color){r,g,b,255}); } return; }
    if(strcmp(word,"log")==0){ char* q1=strchr(cmd,'"'); if(q1){ q1++; char* q2=strchr(q1,'"'); if(q2){ *q2='\0'; AddLog("[Script] %s", q1); } } return; }
    if(strcmp(word,"for")==0){ AddLog("[Script] For loop detected"); return; }
}

void ExecuteScript(const char* scriptName) {
    Script* script=NULL;
    for(int i=0;i<scriptCount;i++) if(strcmp(scripts[i].name, scriptName)==0){ script=&scripts[i]; break; }
    if(!script){ AddLog("Script '%s' not found", scriptName); return; }
    for(int i=0;i<script->lineCount;i++) ExecuteScriptLine(script->lines[i]);
    AddLog("Executed script '%s'", scriptName);
}

void LoadScripts(const char* projectName) {
    // Stub: implement with engine_io if needed
}

void SaveScript(ScriptEditor* ed) {
    // Stub: implement with engine_io if needed
}

bool InitializePython(void) { return true; }
void ExecutePythonScript(const char* scriptName) {}
void FinalizePython(void) {}

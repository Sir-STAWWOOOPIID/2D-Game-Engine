#include "../include/ui.h"
#include "../include/core.h"
#include "../include/visual_editor.h"
#include "../include/script_engine.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <ctype.h>

ScriptEditor openEditors[MAX_OPEN_EDITORS];
int openCount = 0;
int activeEditor = -1;
int caretLine = 0, caretCol = 0;
bool visualEditorOpen = false;
char commandBuffer[CMD_LEN] = {0};
int commandLen = 0;
int logCount = 0;
char logs[LOG_CAP][CMD_LEN];
char suggestions[MAX_SCRIPTS][NAME_LEN];
int suggestionCount = 0;
int highlightedSuggestion = -1;

void AddLog(const char* fmt, ...) {
    char line[CMD_LEN]; 
    va_list args;
    va_start(args, fmt); 
    vsnprintf(line, sizeof(line), fmt, args); 
    va_end(args);

    if(logCount < LOG_CAP) {
        strcpy(logs[logCount++], line);
    } else {
        for(int i = 1; i < LOG_CAP; i++) {
            strcpy(logs[i-1], logs[i]);
        }
        strcpy(logs[LOG_CAP-1], line);
    }
}

void ExecuteCommand(const char* cmd) {
    char word[32]={0};
    if(sscanf(cmd,"%31s",word)!=1){ AddLog("Empty command"); return; }
    if(strcmp(word,"addCircle")==0){ char name[NAME_LEN]; int x,y; float size; int r,g,b; if(sscanf(cmd,"addCircle %31s %d %d %f %d %d %d",name,&x,&y,&size,&r,&g,&b)==7) AddObject(name,SHAPE_CIRCLE,(Vector2){x,y},size,(Color){r,g,b,255}); else AddLog("Usage: addCircle name x y size r g b"); }
    else if(strcmp(word,"addRect")==0){ char name[NAME_LEN]; int x,y; float size; int r,g,b; if(sscanf(cmd,"addRect %31s %d %d %f %d %d %d",name,&x,&y,&size,&r,&g,&b)==7) AddObject(name,SHAPE_RECT,(Vector2){x,y},size,(Color){r,g,b,255}); else AddLog("Usage: addRect name x y size r g b"); }
    else if(strcmp(word,"move")==0){ char name[NAME_LEN]; float dx,dy; if(sscanf(cmd,"move %31s %f %f",name,&dx,&dy)==3){ GameObject *o=FindByName(name); if(o){ o->position=Vector2Add(o->position,(Vector2){dx,dy}); AddLog("Moved '%s'",name);} else AddLog("Error: '%s' not found",name);} else AddLog("Usage: move name dx dy"); }
    else if(strcmp(word,"color")==0){ char name[NAME_LEN]; int r,g,b; if(sscanf(cmd,"color %31s %d %d %d",name,&r,&g,&b)==4){ GameObject *o=FindByName(name); if(o){ o->color=(Color){r,g,b,255}; AddLog("Recolored '%s'",name);} else AddLog("Error: '%s' not found",name);} else AddLog("Usage: color name r g b"); }
    else if(strcmp(word,"update")==0){ char name[NAME_LEN]; if(sscanf(cmd,"update %31s",name)==1) /* TODO: call UpdateProject_local(name) */; else AddLog("Usage: update ProjectName"); }
    else if(strcmp(word,"OpenScript")==0){ char scriptName[NAME_LEN]; if(sscanf(cmd,"OpenScript %31s",scriptName)==1) OpenScriptTab(scriptName); else AddLog("Usage: OpenScript <script>"); }
    else if(strcmp(word,"NewScript")==0){ char scriptName[NAME_LEN]; if(sscanf(cmd,"NewScript %31s",scriptName)==1){ char fullName[NAME_LEN]; if(strstr(scriptName, ".script")==NULL) snprintf(fullName,sizeof(fullName),"%s.script",scriptName); else strncpy(fullName, scriptName, NAME_LEN-1); OpenScriptTab(fullName);} else AddLog("Usage: NewScript <name>"); }
    else if(strcmp(word,"NewPython")==0){ char scriptName[NAME_LEN]; if(sscanf(cmd,"NewPython %31s",scriptName)==1){ char fullName[NAME_LEN]; if(strstr(scriptName, ".py")==NULL) snprintf(fullName,sizeof(fullName),"%s.py",scriptName); else strncpy(fullName, scriptName, NAME_LEN-1); OpenScriptTab(fullName);} else AddLog("Usage: NewPython <name>"); }
    else if(strcmp(word,"RunScript")==0){ char scriptName[NAME_LEN]; if(sscanf(cmd,"RunScript %31s",scriptName)==1) ExecuteScript(scriptName); else AddLog("Usage: RunScript <script>"); }
    else if(strcmp(word,"RunPython")==0){ char scriptName[NAME_LEN]; if(sscanf(cmd,"RunPython %31s",scriptName)==1) ExecutePythonScript(scriptName); else AddLog("Usage: RunPython <script.py>"); }
    else if(strcmp(word,"set")==0){ char varName[NAME_LEN]; float value; if(sscanf(cmd,"set %31s %f",varName,&value)==2){ SetVariable(varName, value); AddLog("Set %s = %.2f", varName, value); } else AddLog("Usage: set varName value"); }
    else if(strcmp(word,"delete")==0){ char name[NAME_LEN]; if(sscanf(cmd,"delete %31s",name)==1){ DeleteGameObject(name); } else AddLog("Usage: delete objectName"); }
    else AddLog("Unknown command: %s",word);
}

void OpenScriptTab(const char* name) {
    for(int i=0;i<openCount;i++){
        if(strcmp(openEditors[i].name,name)==0){ activeEditor=i; return; }
    }
    if(openCount>=MAX_OPEN_EDITORS) return;
    ScriptEditor* ed=&openEditors[openCount];
    strncpy(ed->name,name,NAME_LEN-1);
    ed->lineCount=1;
    ed->scroll=0;
    ed->lines[0][0]='\0';
    ed->isScript = (strstr(name, ".script")!=NULL);
    ed->isPython = (strstr(name, ".py")!=NULL);
    // TODO: Load script content if exists
    activeEditor=openCount++;
    caretLine=0; caretCol=0;
    AddLog("Opened %s '%s'", ed->isPython?"Python script":(ed->isScript?"custom script":"file"), name);
}

float MeasureTextSubstring(const char *text, int n, int fontSize) {
    Font font = GetFontDefault();
    char tmp[LINE_LEN] = {0};
    strncpy(tmp, text, n);
    tmp[n] = '\0';
    return MeasureTextEx(font, tmp, (float)fontSize, 0).x;
}

Color GetScriptTokenColor(const char* token) {
    // Simple example: keywords, numbers, etc.
    if(strcmp(token,"move")==0 || strcmp(token,"setPos")==0 || strcmp(token,"setColor")==0 || strcmp(token,"createCircle")==0 || strcmp(token,"createRect")==0)
        return ACCENT_GREEN;
    if(isdigit(token[0]) || (token[0]=='-' && isdigit(token[1])))
        return ACCENT_ORANGE;
    return TEXT_PRIMARY;
}

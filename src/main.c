#include <raylib.h>
#include <raymath.h>
#include <Python.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <direct.h>
#include <dirent.h>
#include <math.h>
#include <ctype.h>

#define MAX_OBJECTS 128
#define NAME_LEN    32
#define LOG_CAP     3
#define CMD_LEN     256
#define MAX_SCRIPTS 64
#define LINE_LEN    128
#define MAX_OPEN_EDITORS 16
#define VISIBLE_LINES 20
#define MAX_VARIABLES 64

// Define PI if not already defined
#ifndef PI
#define PI 3.14159265358979323846f
#endif

typedef enum { SHAPE_CIRCLE=0, SHAPE_RECT=1 } ShapeType;

typedef struct {
    char name[NAME_LEN];
    Vector2 position;
    float size;
    Color color;
    ShapeType shape;
    bool selected;
} GameObject;

typedef struct {
    char name[NAME_LEN];
    float value;
} Variable;

typedef struct {
    char name[NAME_LEN];
    char lines[128][LINE_LEN];
    int lineCount;
    bool isScript;  // True for our custom scripting language
    bool isPython;  // True for Python scripts
} Script;

typedef struct {
    char name[NAME_LEN];
    char lines[128][LINE_LEN];
    int lineCount;
    int scroll;
    bool isScript;
    bool isPython;
} ScriptEditor;

// Global variables for scripting
static Variable variables[MAX_VARIABLES];
static int variableCount = 0;
static bool scriptRunning = false;
static int currentScriptLine = 0;

static GameObject objects[MAX_OBJECTS];
static int objectCount = 0;

static Script scripts[MAX_SCRIPTS];
static int scriptCount = 0;

static ScriptEditor openEditors[MAX_OPEN_EDITORS];
static int openCount = 0;
static int activeEditor = -1;
static int caretLine=0, caretCol=0;

static bool visualEditorOpen = false;
static int selectedObject = -1; // For visual editor

static char commandBuffer[CMD_LEN] = {0};
static int commandLen = 0;
static char logs[LOG_CAP][CMD_LEN];
static int logCount = 0;
static bool running = false;
static bool dragging = false;
static Vector2 dragOffset = {0};

// --- Auto-suggest ---
static char suggestions[MAX_SCRIPTS][NAME_LEN];
static int suggestionCount = 0;
static int highlightedSuggestion = -1;

// Color scheme
static const Color BG_DARK = {25, 25, 30, 255};
static const Color BG_MEDIUM = {35, 35, 40, 255};
static const Color BG_LIGHT = {45, 45, 50, 255};
static const Color ACCENT_BLUE = {64, 128, 255, 255};
static const Color ACCENT_GREEN = {64, 255, 128, 255};
static const Color ACCENT_RED = {255, 64, 64, 255};
static const Color ACCENT_ORANGE = {255, 165, 0, 255};
static const Color ACCENT_PURPLE = {160, 64, 255, 255};
static const Color TEXT_PRIMARY = {255, 255, 255, 255};
static const Color TEXT_SECONDARY = {200, 200, 200, 255};

// Forward declarations
static void ExecuteCommand(const char* cmd);
static void ExecuteScriptLine(const char* line);
static float GetVariable(const char* name);
static void SetVariable(const char* name, float value);
static char currentProject[NAME_LEN] = "Default"; // Active project name


// Measure width of first 'n' characters of a string using Raylib font
float MeasureTextSubstring(const char *text, int n, int fontSize){
    Font font = GetFontDefault();
    char tmp[LINE_LEN] = {0};
    strncpy(tmp, text, n);
    tmp[n] = '\0';
    return MeasureTextEx(font, tmp, (float)fontSize, 0).x;
}

// --- Logging ---
static void AddLog(const char* fmt, ...) {
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

// Initialize Python and set up game API
static bool InitializePython() {
    Py_Initialize();
    if (!Py_IsInitialized()) {
        AddLog("Failed to initialize Python");
        return false;
    }
    
    // Add game functions to Python environment
    PyRun_SimpleString(
        "# Game Engine API\n"
        "game_objects = {}\n"
        "game_variables = {}\n"
        "game_commands = []\n"
        "\n"
        "def move_object(name, dx, dy):\n"
        "    cmd = f'move {name} {dx} {dy}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Moving {name} by {dx}, {dy}')\n"
        "\n"
        "def set_position(name, x, y):\n"
        "    cmd = f'setPos {name} {x} {y}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Setting {name} position to {x}, {y}')\n"
        "\n"
        "def set_color(name, r, g, b):\n"
        "    cmd = f'color {name} {r} {g} {b}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Setting {name} color to {r}, {g}, {b}')\n"
        "\n"
        "def create_circle(name, x, y, size, r, g, b):\n"
        "    cmd = f'addCircle {name} {x} {y} {size} {r} {g} {b}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Creating circle {name}')\n"
        "\n"
        "def create_rect(name, x, y, size, r, g, b):\n"
        "    cmd = f'addRect {name} {x} {y} {size} {r} {g} {b}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Creating rectangle {name}')\n"
        "\n"
        "def delete_obj(name):\n"
        "    cmd = f'delete {name}'\n"
        "    game_commands.append(cmd)\n"
        "    print(f'[Python] Deleting object: {name}')\n"
        "\n"
        "def log_message(msg):\n"
        "    print(f'[Python] {msg}')\n"
        "\n"
        "import math\n"
        "import time\n"
    );
    
    AddLog("Python initialized with game API");
    return true;
}

// --- Variable System ---
static float GetVariable(const char* name) {
    for(int i = 0; i < variableCount; i++) {
        if(strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return 0.0f; // Default value
}

static void SetVariable(const char* name, float value) {
    // Check if variable exists
    for(int i = 0; i < variableCount; i++) {
        if(strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    
    // Create new variable
    if(variableCount < MAX_VARIABLES) {
        strncpy(variables[variableCount].name, name, NAME_LEN-1);
        variables[variableCount].value = value;
        variableCount++;
    }
}

// --- Simple Expression Evaluator ---
static float EvaluateExpression(const char* expr) {
    char cleanExpr[256] = {0};
    strncpy(cleanExpr, expr, 255);
    
    // Remove spaces
    char* src = cleanExpr;
    char* dst = cleanExpr;
    while(*src) {
        if(*src != ' ') *dst++ = *src;
        src++;
    }
    *dst = '\0';
    
    // Simple evaluation - handles basic math and variables
    if(isdigit(cleanExpr[0]) || cleanExpr[0] == '-') {
        return atof(cleanExpr);
    }
    
    // Check for simple operations
    char* plus = strchr(cleanExpr, '+');
    char* minus = strchr(cleanExpr, '-');
    char* mult = strchr(cleanExpr, '*');
    char* div = strchr(cleanExpr, '/');
    
    if(plus) {
        *plus = '\0';
        return EvaluateExpression(cleanExpr) + EvaluateExpression(plus + 1);
    }
    if(minus && minus != cleanExpr) { // Not negative number
        *minus = '\0';
        return EvaluateExpression(cleanExpr) - EvaluateExpression(minus + 1);
    }
    if(mult) {
        *mult = '\0';
        return EvaluateExpression(cleanExpr) * EvaluateExpression(mult + 1);
    }
    if(div) {
        *div = '\0';
        float denominator = EvaluateExpression(div + 1);
        return denominator != 0 ? EvaluateExpression(cleanExpr) / denominator : 0;
    }
    
    // Check for math functions
    if(strncmp(cleanExpr, "sin(", 4) == 0) {
        char* end = strchr(cleanExpr, ')');
        if(end) {
            *end = '\0';
            return sin(EvaluateExpression(cleanExpr + 4) * PI / 180.0f);
        }
    }
    if(strncmp(cleanExpr, "cos(", 4) == 0) {
        char* end = strchr(cleanExpr, ')');
        if(end) {
            *end = '\0';
            return cos(EvaluateExpression(cleanExpr + 4) * PI / 180.0f);
        }
    }
    
    // Must be a variable
    return GetVariable(cleanExpr);
}

// --- Objects ---
static GameObject* FindByName(const char* name){
    for(int i = 0; i < objectCount; i++) if(strcmp(objects[i].name, name) == 0) return &objects[i];
    return NULL;
}

static bool NameExists(const char* name){ return FindByName(name)!=NULL; }

static void AddObject(const char* name, ShapeType shape, Vector2 pos, float size, Color col){
    if(objectCount >= MAX_OBJECTS){ AddLog("Error: object limit reached"); return; }
    if(NameExists(name)){ AddLog("Error: name '%s' exists", name); return; }
    GameObject o = {0};
    strncpy(o.name, name, NAME_LEN-1);
    o.position = pos; o.size = (size <= 0 ? 1 : size); o.color = col; o.shape = shape; o.selected = false;
    objects[objectCount++] = o;
    AddLog("Added %s '%s'", shape==SHAPE_CIRCLE?"circle":"rect", name);
}

static void DeleteObject(const char* name){
    for(int i=0;i<objectCount;i++){
        if(strcmp(objects[i].name,name)==0){
            for(int j=i+1;j<objectCount;j++) objects[j-1]=objects[j];
            objectCount--; AddLog("Deleted '%s'",name); return;
        }
    }
    AddLog("Error: '%s' not found",name);
}

// --- Project / Script files ---
static void CreateDirIfNotExist(const char* path){ _mkdir(path); }

static void UpdateProject(const char* name){
    strncpy(currentProject, name, NAME_LEN-1);
    currentProject[NAME_LEN-1] = '\0'; // safe terminate
    char path[256]; snprintf(path,sizeof(path),"./Projects/%s",name);
    CreateDirIfNotExist("./Projects"); CreateDirIfNotExist(path);
    char mainFile[256]; snprintf(mainFile,sizeof(mainFile),"%s/main.game",path);
    FILE *f = fopen(mainFile,"w");
    if(f){ 
        fprintf(f,"# %s Project\n",name);
        for(int i=0;i<objectCount;i++){ GameObject* o=&objects[i];
            fprintf(f,"%s %s %.0f %.0f %.0f %d %d %d\n",
                o->shape==SHAPE_CIRCLE?"circle":"rect", o->name, o->position.x,o->position.y,o->size,
                o->color.r,o->color.g,o->color.b);
        }
        fclose(f);
    }
    char scriptsDir[256]; snprintf(scriptsDir,sizeof(scriptsDir),"%s/scripts",path);
    CreateDirIfNotExist(scriptsDir);
    AddLog("Project '%s' updated", name);
}

static void LoadScripts(const char* projectName){
    scriptCount = 0;
    char path[256]; snprintf(path,sizeof(path),"./Projects/%s/scripts",projectName);
    DIR *dir = opendir(path);
    if(!dir){ AddLog("No scripts folder"); return; }
    struct dirent *entry;
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) continue;
        if(scriptCount >= MAX_SCRIPTS) break;
        Script *s = &scripts[scriptCount]; s->lineCount=0;
        strncpy(s->name, entry->d_name, NAME_LEN-1);
        
        // Check file type
        s->isScript = (strstr(entry->d_name, ".script") != NULL);
        s->isPython = (strstr(entry->d_name, ".py") != NULL);
        
        char filePath[256]; snprintf(filePath,sizeof(filePath),"%s/%s", path, entry->d_name);
        FILE *f=fopen(filePath,"r"); if(!f) continue;
        while(fgets(s->lines[s->lineCount], LINE_LEN, f) && s->lineCount<128){
            char *p=strchr(s->lines[s->lineCount],'\n'); if(p)*p=0;
            s->lineCount++;
        }
        fclose(f); scriptCount++;
        
        const char* typeStr = s->isPython ? "Python script" : (s->isScript ? "custom script" : "file");
        AddLog("Loaded %s '%s'", typeStr, s->name);
    }
    closedir(dir);
}

static void SaveScript(ScriptEditor* ed){
    char scriptsDir[256];
    snprintf(scriptsDir, sizeof(scriptsDir), "./Projects/%s/scripts", currentProject);
    CreateDirIfNotExist("./Projects");
    CreateDirIfNotExist(scriptsDir);

    char path[256];
    snprintf(path, sizeof(path), "%s/%s", scriptsDir, ed->name);

    FILE *f = fopen(path,"w");
    if(!f){ 
        AddLog("Failed to save '%s'", ed->name); 
        return; 
    }
    for(int i=0;i<ed->lineCount;i++) {
        fprintf(f,"%s\n", ed->lines[i]);
    }
    fclose(f);
    AddLog("Saved '%s' in project '%s'", ed->name, currentProject);
}

// --- Enhanced Script Execution ---
static void ExecuteScriptLine(const char* line) {
    if(strlen(line) == 0 || line[0] == '#') return;
    
    char cmd[CMD_LEN];
    strncpy(cmd, line, CMD_LEN-1);
    cmd[CMD_LEN-1] = '\0';
    
    char word[64];
    if(sscanf(cmd, "%63s", word) != 1) return;
    
    // Variable assignment: var = expression
    char* equals = strchr(cmd, '=');
    if(equals) {
        *equals = '\0';
        char varName[64];
        sscanf(cmd, "%63s", varName);
        float value = EvaluateExpression(equals + 1);
        SetVariable(varName, value);
        AddLog("[Script] %s = %.2f", varName, value);
        return;
    }
    
    // Move object: move objectName deltaX deltaY
    if(strcmp(word, "move") == 0) {
        char objName[NAME_LEN];
        char dxExpr[64], dyExpr[64];
        if(sscanf(cmd, "move %31s %63s %63s", objName, dxExpr, dyExpr) == 3) {
            GameObject* obj = FindByName(objName);
            if(obj) {
                float dx = EvaluateExpression(dxExpr);
                float dy = EvaluateExpression(dyExpr);
                obj->position = Vector2Add(obj->position, (Vector2){dx, dy});
            }
        }
        return;
    }
    
    // Set position: setPos objectName x y
    if(strcmp(word, "setPos") == 0) {
        char objName[NAME_LEN];
        char xExpr[64], yExpr[64];
        if(sscanf(cmd, "setPos %31s %63s %63s", objName, xExpr, yExpr) == 3) {
            GameObject* obj = FindByName(objName);
            if(obj) {
                float x = EvaluateExpression(xExpr);
                float y = EvaluateExpression(yExpr);
                obj->position = (Vector2){x, y};
            }
        }
        return;
    }
    
    // Set color: setColor objectName r g b
    if(strcmp(word, "setColor") == 0) {
        char objName[NAME_LEN];
        char rExpr[64], gExpr[64], bExpr[64];
        if(sscanf(cmd, "setColor %31s %63s %63s %63s", objName, rExpr, gExpr, bExpr) == 4) {
            GameObject* obj = FindByName(objName);
            if(obj) {
                int r = (int)EvaluateExpression(rExpr);
                int g = (int)EvaluateExpression(gExpr);
                int b = (int)EvaluateExpression(bExpr);
                obj->color = (Color){r, g, b, 255};
            }
        }
        return;
    }
    
    // Create circle: createCircle name x y size r g b
    if(strcmp(word, "createCircle") == 0) {
        char objName[NAME_LEN];
        char args[6][64];
        if(sscanf(cmd, "createCircle %31s %63s %63s %63s %63s %63s %63s", 
                 objName, args[0], args[1], args[2], args[3], args[4], args[5]) == 7) {
            float x = EvaluateExpression(args[0]);
            float y = EvaluateExpression(args[1]);
            float size = EvaluateExpression(args[2]);
            int r = (int)EvaluateExpression(args[3]);
            int g = (int)EvaluateExpression(args[4]);
            int b = (int)EvaluateExpression(args[5]);
            AddObject(objName, SHAPE_CIRCLE, (Vector2){x, y}, size, (Color){r, g, b, 255});
        }
        return;
    }
    
    // Create rectangle: createRect name x y size r g b
    if(strcmp(word, "createRect") == 0) {
        char objName[NAME_LEN];
        char args[6][64];
        if(sscanf(cmd, "createRect %31s %63s %63s %63s %63s %63s %63s", 
                 objName, args[0], args[1], args[2], args[3], args[4], args[5]) == 7) {
            float x = EvaluateExpression(args[0]);
            float y = EvaluateExpression(args[1]);
            float size = EvaluateExpression(args[2]);
            int r = (int)EvaluateExpression(args[3]);
            int g = (int)EvaluateExpression(args[4]);
            int b = (int)EvaluateExpression(args[5]);
            AddObject(objName, SHAPE_RECT, (Vector2){x, y}, size, (Color){r, g, b, 255});
        }
        return;
    }
    
    // Log message: log "message"
    if(strcmp(word, "log") == 0) {
        char* quote1 = strchr(cmd, '"');
        if(quote1) {
            quote1++; // Skip opening quote
            char* quote2 = strchr(quote1, '"');
            if(quote2) {
                *quote2 = '\0';
                AddLog("[Script] %s", quote1);
            }
        }
        return;
    }
    
    // For loop: for var start end
    if(strcmp(word, "for") == 0) {
        // This would need more complex parsing for full implementation
        // For now, just log it
        AddLog("[Script] For loop detected");
        return;
    }
}

static void ExecuteScript(const char* scriptName) {
    // Find the script
    Script *script = NULL;
    for(int i = 0; i < scriptCount; i++) {
        if(strcmp(scripts[i].name, scriptName) == 0) {
            script = &scripts[i];
            break;
        }
    }
    
    if(!script) {
        AddLog("Script '%s' not found", scriptName);
        return;
    }
    
    // Execute all lines
    for(int i = 0; i < script->lineCount; i++) {
        ExecuteScriptLine(script->lines[i]);
    }
    
    AddLog("Executed script '%s'", scriptName);
}

// Execute Python script and capture commands
    static void ExecutePythonScript(const char* scriptName) {
        // Build the full file path
        char scriptPath[256];
        snprintf(scriptPath, sizeof(scriptPath), "./Projects/%s/scripts/%s", currentProject, scriptName);
        
        // Check if file exists
        FILE* testFile = fopen(scriptPath, "r");
        if(!testFile) {
            AddLog("Python script file '%s' not found at path: %s", scriptName, scriptPath);
            return;
        }
        fclose(testFile);
        
        // Clear previous commands
        PyRun_SimpleString("game_commands.clear()");
        
        // Read the file content
        FILE* file = fopen(scriptPath, "r");
        if(!file) {
            AddLog("Failed to open Python script '%s'", scriptName);
            return;
        }
        
        char pythonCode[8192] = {0}; // Adjust size as needed
        char line[256];
        while(fgets(line, sizeof(line), file)) {
            strcat(pythonCode, line);
        }
        fclose(file);
        
        // Execute the Python script
        if(PyRun_SimpleString(pythonCode) != 0) {
            AddLog("Python script execution failed for '%s'", scriptName);
            return;
        }
        
        // Get and execute the generated commands
        PyObject* commandsList = PyObject_GetAttrString(PyImport_AddModule("__main__"), "game_commands");
        if(commandsList && PyList_Check(commandsList)) {
            Py_ssize_t size = PyList_Size(commandsList);
            for(Py_ssize_t i = 0; i < size; i++) {
                PyObject* item = PyList_GetItem(commandsList, i);
                if(PyUnicode_Check(item)) {
                    const char* cmd = PyUnicode_AsUTF8(item);
                    if(cmd) {
                        ExecuteCommand(cmd);
                    }
                }
            }
        }
        
        AddLog("Executed Python script '%s' from project '%s'", scriptName, currentProject);
    }

// --- Editor Utilities ---
static void OpenScriptTab(const char* name){
    for(int i=0;i<openCount;i++){ if(strcmp(openEditors[i].name,name)==0){ activeEditor=i; return; } }
    if(openCount>=MAX_OPEN_EDITORS) return;
    ScriptEditor* ed=&openEditors[openCount];
    strncpy(ed->name,name,NAME_LEN-1); ed->lineCount=1; ed->scroll=0;
    strcpy(ed->lines[0], "");
    
    // Check file type
    ed->isScript = (strstr(name, ".script") != NULL);
    ed->isPython = (strstr(name, ".py") != NULL);
    
    // Load from existing script if found
    for(int i=0;i<scriptCount;i++) if(strcmp(scripts[i].name,name)==0){
        ed->lineCount = scripts[i].lineCount;
        ed->isScript = scripts[i].isScript;
        ed->isPython = scripts[i].isPython;
        for(int j=0;j<scripts[i].lineCount;j++) strcpy(ed->lines[j],scripts[i].lines[j]);
        break;
    }
    activeEditor=openCount;
    openCount++;
    caretLine = 0;
    caretCol = 0;
    const char* typeStr = ed->isPython ? "Python script" : (ed->isScript ? "custom script" : "file");
    AddLog("Opened %s '%s'", typeStr, name);
}

// --- Command execution ---
static void ExecuteCommand(const char* cmd){
    char word[32]={0}; if(sscanf(cmd,"%31s",word)!=1){AddLog("Empty command"); return;}
    if(strcmp(word,"addCircle")==0){
        char name[NAME_LEN]; int x,y; float size; int r,g,b;
        if(sscanf(cmd,"addCircle %31s %d %d %f %d %d %d",name,&x,&y,&size,&r,&g,&b)==7)
            AddObject(name,SHAPE_CIRCLE,(Vector2){x,y},size,(Color){r,g,b,255});
        else AddLog("Usage: addCircle name x y size r g b");
    }
    else if(strcmp(word,"addRect")==0){
        char name[NAME_LEN]; int x,y; float size; int r,g,b;
        if(sscanf(cmd,"addRect %31s %d %d %f %d %d %d",name,&x,&y,&size,&r,&g,&b)==7)
            AddObject(name,SHAPE_RECT,(Vector2){x,y},size,(Color){r,g,b,255});
        else AddLog("Usage: addRect name x y size r g b");
    }
    else if(strcmp(word,"move")==0){
        char name[NAME_LEN]; float dx,dy;
        if(sscanf(cmd,"move %31s %f %f",name,&dx,&dy)==3){
            GameObject *o=FindByName(name);
            if(o){ o->position=Vector2Add(o->position,(Vector2){dx,dy}); AddLog("Moved '%s'",name);}
            else AddLog("Error: '%s' not found",name);
        } else AddLog("Usage: move name dx dy");
    }
    else if(strcmp(word,"color")==0){
        char name[NAME_LEN]; int r,g,b;
        if(sscanf(cmd,"color %31s %d %d %d",name,&r,&g,&b)==4){
            GameObject *o=FindByName(name);
            if(o){ o->color=(Color){r,g,b,255}; AddLog("Recolored '%s'",name);}
            else AddLog("Error: '%s' not found",name);
        } else AddLog("Usage: color name r g b");
    }
    else if(strcmp(word,"update")==0){
        char name[NAME_LEN]; if(sscanf(cmd,"update %31s",name)==1) UpdateProject(name);
        else AddLog("Usage: update ProjectName");
    }
    else if(strcmp(word,"OpenScript")==0){
        char scriptName[NAME_LEN];
        if(sscanf(cmd,"OpenScript %31s",scriptName)==1) OpenScriptTab(scriptName);
        else AddLog("Usage: OpenScript <script>");
    }
    else if(strcmp(word,"NewScript")==0){
        char scriptName[NAME_LEN];
        if(sscanf(cmd,"NewScript %31s",scriptName)==1) {
            // Add .script extension if not present
            char fullName[NAME_LEN];
            if(strstr(scriptName, ".script") == NULL) {
                snprintf(fullName, sizeof(fullName), "%s.script", scriptName);
            } else {
                strncpy(fullName, scriptName, NAME_LEN-1);
            }
            OpenScriptTab(fullName);
        } else AddLog("Usage: NewScript <name>");
    }
    else if(strcmp(word,"NewPython")==0){
        char scriptName[NAME_LEN];
        if(sscanf(cmd,"NewPython %31s",scriptName)==1) {
            char fullName[NAME_LEN];
            if(strstr(scriptName, ".py") == NULL) {
                snprintf(fullName, sizeof(fullName), "%s.py", scriptName);
            } else {
                strncpy(fullName, scriptName, NAME_LEN-1);
            }
            OpenScriptTab(fullName);
        } else AddLog("Usage: NewPython <name>");
    }
    else if(strcmp(word,"RunScript")==0){
        char scriptName[NAME_LEN];
        if(sscanf(cmd,"RunScript %31s",scriptName)==1) ExecuteScript(scriptName);
        else AddLog("Usage: RunScript <script>");
    }
    else if(strcmp(word,"RunPython")==0){
        char scriptName[NAME_LEN];
        if(sscanf(cmd,"RunPython %31s",scriptName)==1) ExecutePythonScript(scriptName);
        else AddLog("Usage: RunPython <script.py>");
    }
    else if(strcmp(word,"set")==0){
        char varName[NAME_LEN]; float value;
        if(sscanf(cmd,"set %31s %f",varName,&value)==2) {
            SetVariable(varName, value);
            AddLog("Set %s = %.2f", varName, value);
        } else AddLog("Usage: set varName value");
    }
    else if(strcmp(word,"delete")==0){
        char name[NAME_LEN];
        if(sscanf(cmd,"delete %31s",name)==1) {
            DeleteObject(name);
        } else {
            AddLog("Usage: delete objectName");
        }
    }
    else AddLog("Unknown command: %s",word);
}

// --- Mouse/Object utilities ---
static void ClearSelection(void){for(int i=0;i<objectCount;i++)objects[i].selected=false;}
static bool PointInObject(Vector2 p,const GameObject* o){
    if(o->shape==SHAPE_CIRCLE) return Vector2Distance(p,o->position)<=o->size;
    return (p.x>=o->position.x-o->size && p.x<=o->position.x+o->size && p.y>=o->position.y-o->size && p.y<=o->position.y+o->size);
}

// Simple syntax highlighting for our scripting language
static Color GetScriptTokenColor(const char* token) {
    // Keywords
    const char* keywords[] = {"move", "setPos", "setColor", "createCircle", "createRect", 
                             "log", "for", "if", "while", "end"};
    int keywordCount = sizeof(keywords) / sizeof(keywords[0]);
    
    for(int i = 0; i < keywordCount; i++) {
        if(strcmp(token, keywords[i]) == 0) {
            return ACCENT_BLUE;
        }
    }
    
    // Numbers
    if(isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
        return ACCENT_ORANGE;
    }
    
    // Strings
    if(token[0] == '"') {
        return ACCENT_GREEN;
    }
    
    // Variables (common names)
    if(strcmp(token, "x") == 0 || strcmp(token, "y") == 0 || 
       strcmp(token, "i") == 0 || strcmp(token, "angle") == 0) {
        return ACCENT_PURPLE;
    }
    
    return TEXT_PRIMARY;
}

// Add Python syntax highlighting
static Color GetPythonTokenColor(const char* token) {
    // Python keywords - expanded list
    const char* keywords[] = {
        "and", "assert", "break", "class", "continue", "def",
        "del", "elif", "else", "except", "exec", "finally",
        "for", "from", "global", "if", "import", "in",
        "is", "lambda", "not", "or", "pass", "print",
        "raise", "return", "try", "while", "yield",
        "None", "True", "False"
    };
    int keywordCount = sizeof(keywords) / sizeof(keywords[0]);
    
    for(int i = 0; i < keywordCount; i++) {
        if(strcmp(token, keywords[i]) == 0) {
            return ACCENT_BLUE;
        }
    }
    
    // Python built-in functions
    const char* builtins[] = {"len", "range", "str", "int", "float", "list", "dict", "open", "abs", "min", "max"};
    int builtinCount = sizeof(builtins) / sizeof(builtins[0]);
    
    for(int i = 0; i < builtinCount; i++) {
        if(strcmp(token, builtins[i]) == 0) {
            return ACCENT_PURPLE;
        }
    }
    
    // Game API functions
    const char* gameAPI[] = {"move_object", "set_position", "set_color", "create_circle", "create_rect", "log_message"};
    int gameAPICount = sizeof(gameAPI) / sizeof(gameAPI[0]);
    
    for(int i = 0; i < gameAPICount; i++) {
        if(strcmp(token, gameAPI[i]) == 0) {
            return ACCENT_GREEN;
        }
    }
    
    // Numbers
    if(isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
        return ACCENT_ORANGE;
    }
    
    // Strings
    if(token[0] == '"' || token[0] == '\'') {
        return ACCENT_GREEN;
    }
    
    return TEXT_PRIMARY;
}

// --- Main ---
int main(void){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200,800,"2D Game Engine with Python Integration");
    SetTargetFPS(60);
    // Load custom font (try multiple paths)
    Font customFont = LoadFont("C:/Fonts/Codey/static/SourceCodePro-Bold.ttf");
    if (customFont.texture.id == 0) {
        // Try alternative paths
        customFont = LoadFont("./fonts/SourceCodePro-Bold.ttf");
    }
    if (customFont.texture.id == 0) {
        // Fallback to default if font fails to load
        customFont = GetFontDefault();
        AddLog("Failed to load custom font, using default");
    } else {
        AddLog("Custom font loaded successfully");
    }

    // Initialize Python
    if(!InitializePython()) {
        AddLog("Warning: Python initialization failed");
    }

    AddObject("player",SHAPE_CIRCLE,(Vector2){500,300},40,ACCENT_RED);
    AddObject("box",SHAPE_RECT,(Vector2){700,350},50,ACCENT_BLUE);

    CreateDirIfNotExist("./Projects/Default/scripts");
    LoadScripts(currentProject);

    // Initialize some example variables
    SetVariable("speed", 5.0f);
    SetVariable("angle", 0.0f);

    Rectangle playButton={(float)GetScreenWidth()-120,10,100,40};

    int editorCharY = 18;

    while(!WindowShouldClose()){
        const int screenW = GetScreenWidth();
        const int screenH = GetScreenHeight();
        const int commandBarH=40;
        const int logBarH=80;
        const int tabBarH=35;
        const int leftPanelW=220;
        const int bottomH=commandBarH+logBarH;

        Vector2 mouse=GetMousePosition();

        // --- Input handling ---
        int ch = GetCharPressed();
        while(ch > 0) {
            if(activeEditor >= 0 && !visualEditorOpen) {
                // Script editor input
                ScriptEditor *ed = &openEditors[activeEditor];
                if(caretLine >= ed->lineCount) caretLine = ed->lineCount - 1;
                if(caretLine < 0) caretLine = 0;
                caretCol = Clamp(caretCol, 0, strlen(ed->lines[caretLine]));
                
                if(ch >= 32 && ch <= 125){
                    int len = strlen(ed->lines[caretLine]);
                    if(len < LINE_LEN-1 && caretCol <= len){
                        // Shift characters to the right, including null terminator
                        memmove(&ed->lines[caretLine][caretCol+1], 
                                &ed->lines[caretLine][caretCol], 
                                len - caretCol + 1);
                        // Insert the new character
                        ed->lines[caretLine][caretCol] = (char)ch;
                        caretCol++;
                        if(caretLine >= ed->scroll + VISIBLE_LINES) ed->scroll++;
                    }
                }
            } else {
                // Visual editor or no editor - input goes to command bar
                if(ch >= 32 && ch <= 125 && commandLen < CMD_LEN-1){
                    commandBuffer[commandLen++] = (char)ch;
                    commandBuffer[commandLen] = '\0';
                }
            }
            ch = GetCharPressed();
        }

        // --- Backspace / Enter ---
        if(activeEditor >= 0 && !visualEditorOpen){
            ScriptEditor *ed = &openEditors[activeEditor];
            if(IsKeyPressed(KEY_BACKSPACE) && caretCol>0){
                int len=strlen(ed->lines[caretLine]);
                for(int i=caretCol-1;i<len;i++) ed->lines[caretLine][i]=ed->lines[caretLine][i+1];
                caretCol--;
            }
            if(IsKeyPressed(KEY_ENTER)){
                if(ed->lineCount<128){
                    for(int i=ed->lineCount;i>caretLine+1;i--) strcpy(ed->lines[i],ed->lines[i-1]);
                    ed->lines[caretLine+1][0]=0;
                    caretLine++; caretCol=0; ed->lineCount++;
                    if(caretLine >= ed->scroll + VISIBLE_LINES) ed->scroll++;
                }
            }
            // Arrow keys
            if(IsKeyPressed(KEY_UP)){ if(caretLine>0) caretLine--; caretCol=Clamp(caretCol,0,(int)strlen(ed->lines[caretLine])); if(caretLine<ed->scroll) ed->scroll--; }
            if(IsKeyPressed(KEY_DOWN)){ if(caretLine<ed->lineCount-1) caretLine++; caretCol=Clamp(caretCol,0,(int)strlen(ed->lines[caretLine])); if(caretLine>=ed->scroll+VISIBLE_LINES) ed->scroll++; }
            if(IsKeyPressed(KEY_LEFT) && caretCol>0) caretCol--;
            if(IsKeyPressed(KEY_RIGHT) && caretCol<(int)strlen(ed->lines[caretLine])) caretCol++;
            
            // Ctrl+S to save
            if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) SaveScript(ed);
            
            // F5 to run script
            if(IsKeyPressed(KEY_F5)) {
                SaveScript(ed);
                if(ed->isPython) {
                    ExecutePythonScript(ed->name);
                } else if(ed->isScript) {
                    ExecuteScript(ed->name);
                }
            }
        } else {
            // Visual editor or command bar input
            if(IsKeyPressed(KEY_BACKSPACE) && commandLen>0){ commandLen--; commandBuffer[commandLen]='\0'; }
            if(IsKeyPressed(KEY_ENTER) && commandLen>0){ ExecuteCommand(commandBuffer); commandLen=0; commandBuffer[0]='\0'; }
        }

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            int totalTabs = openCount + 1;
            float tabW = (float)screenW / totalTabs;
            
            // Check visual editor tab
            Rectangle visualTab = {0, 0, tabW, tabBarH};
            if(CheckCollisionPointRec(mouse, visualTab)){
                visualEditorOpen = true;
                activeEditor = -1;
            } else {
                // Check script editor tabs
                for(int i=0;i<openCount;i++){
                    Rectangle tab={(i+1)*tabW,0,tabW,tabBarH};
                    if(CheckCollisionPointRec(mouse, tab)){
                        activeEditor = i;
                        visualEditorOpen = false;
                        caretLine = 0;
                        caretCol = 0;
                        break;
                    }
                }
            }
        }

        // --- Auto-suggest ---
        suggestionCount=0; highlightedSuggestion=-1;
        if(strncmp(commandBuffer,"OpenScript ",11)==0){
            const char* partial = commandBuffer+11;
            for(int i=0;i<scriptCount;i++){
                if(strncmp(scripts[i].name, partial, strlen(partial))==0){
                    strncpy(suggestions[suggestionCount], scripts[i].name, NAME_LEN-1);
                    suggestionCount++; if(suggestionCount>=MAX_SCRIPTS) break;
                }
            }
        }

        // --- Object dragging ---
        Rectangle canvas = {(float)leftPanelW, tabBarH, (float)(screenW-leftPanelW), (float)(screenH-bottomH-tabBarH)};
        if(CheckCollisionPointRec(mouse, canvas)){
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                dragging=false; int picked=-1;
                for(int i=objectCount-1;i>=0;i--) if(PointInObject(mouse,&objects[i])){picked=i; break;}
                if(picked>=0){ ClearSelection(); objects[picked].selected=true; dragOffset=Vector2Subtract(objects[picked].position,mouse); dragging=true; AddLog("Selected '%s'",objects[picked].name);}
                else ClearSelection();
            }
            if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && dragging) for(int i=0;i<objectCount;i++) if(objects[i].selected) objects[i].position=Vector2Add(mouse,dragOffset);
            if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging=false;
        } else if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging=false;

        // --- Run scripts ---
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse,playButton)) running=!running;
        if(running){
            for(int s=0;s<scriptCount;s++){
                Script *sc=&scripts[s];
                if(sc->isScript) {
                    // Skip scripts in auto-run mode - use F5 or RunScript command instead
                    continue;
                } else {
                    for(int l=0;l<sc->lineCount;l++) ExecuteScriptLine(sc->lines[l]);
                }
            }
        }

        // Update variables for animation
        if(running) {
            float currentAngle = GetVariable("angle");
            SetVariable("angle", currentAngle + 2.0f);
            if(currentAngle > 360) SetVariable("angle", 0);
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(BG_DARK);

        // Calculate tab width (including visual editor tab)
        int totalTabs = openCount + 1; // +1 for visual editor
        float tabW = (float)screenW / totalTabs;

        // Draw visual editor tab first
        Rectangle visualTab = {0, 0, tabW, tabBarH};
        Color visualTabColor = visualEditorOpen ? BG_LIGHT : BG_MEDIUM;
        DrawRectangleRec(visualTab, visualTabColor);
        if(visualEditorOpen) {
            DrawRectangle(visualTab.x, visualTab.y + visualTab.height - 3, visualTab.width, 3, ACCENT_BLUE);
        }
        DrawTextEx(customFont, "Visual Editor", (Vector2){visualTab.x+10, visualTab.y+8}, 16, 0, TEXT_PRIMARY);

        // Draw script editor tabs
        for(int i=0;i<openCount;i++){
            Rectangle tab={(i+1)*tabW,0,tabW,tabBarH}; // +1 to account for visual editor tab
            Color tabColor = (i == activeEditor && !visualEditorOpen) ? BG_LIGHT : BG_MEDIUM;
            DrawRectangleRec(tab, tabColor);
            if(i == activeEditor && !visualEditorOpen) {
                DrawRectangle(tab.x, tab.y + tab.height - 3, tab.width, 3, ACCENT_BLUE);
            }
            
            // Draw script indicators and names (your existing code)
            if(openEditors[i].isPython) {
                DrawTextEx(customFont, "Python", (Vector2){tab.x+5, tab.y+5}, 10, 0, ACCENT_GREEN);
                DrawTextEx(customFont, openEditors[i].name, (Vector2){tab.x+45, tab.y+8}, 16, 0, TEXT_PRIMARY);
            } else if(openEditors[i].isScript) {
                DrawTextEx(customFont, "Script", (Vector2){tab.x+5, tab.y+5}, 10, 0, ACCENT_PURPLE);
                DrawTextEx(customFont, openEditors[i].name, (Vector2){tab.x+45, tab.y+8}, 16, 0, TEXT_PRIMARY);
            } else {
                DrawTextEx(customFont, openEditors[i].name, (Vector2){tab.x+10, tab.y+8}, 16, 0, TEXT_PRIMARY);
            }
            
            if(i > 0 || true) { // Always draw separator after visual editor tab
                DrawLine(tab.x, tab.y, tab.x, tab.y + tab.height, BG_DARK);
            }
        }

        // Enhanced Left panel with variables
        DrawRectangle(0,tabBarH,(float)leftPanelW,(float)(screenH-bottomH-tabBarH),BG_MEDIUM);
        DrawRectangle(leftPanelW-1,tabBarH,1,(float)(screenH-bottomH-tabBarH),BG_LIGHT);
        
        int yOffset = tabBarH+15;
        DrawTextEx(customFont, "Objects:", (Vector2){15, yOffset}, 18, 0, TEXT_PRIMARY);
        yOffset += 30;
        
        for(int i=0;i<objectCount && i<8;i++){  // Limit to 8 objects for space
            Color objColor = objects[i].selected ? ACCENT_BLUE : TEXT_SECONDARY;
            Rectangle objRect = {10, yOffset+i*25, leftPanelW-20, 22};
            if(objects[i].selected) {
                DrawRectangleRounded(objRect, 0.2f, 8, (Color){ACCENT_BLUE.r, ACCENT_BLUE.g, ACCENT_BLUE.b, 40});
            }
            DrawTextEx(customFont, objects[i].name, (Vector2){15, yOffset+5+i*25}, 16, 0, objColor);
            
            if(objects[i].shape == SHAPE_CIRCLE) {
                DrawCircle(leftPanelW-25, yOffset+13+i*25, 6, objects[i].color);
            } else {
                DrawRectangle(leftPanelW-31, yOffset+7+i*25, 12, 12, objects[i].color);
            }
        }
        
        yOffset += objectCount * 25 + 20;
        DrawTextEx(customFont, "Variables:", (Vector2){15, yOffset}, 18, 0, TEXT_PRIMARY);
        yOffset += 25;
        
        for(int i=0;i<variableCount && i<6;i++){  // Limit to 6 variables for space
            char varText[64];
            snprintf(varText, sizeof(varText), "%s: %.1f", variables[i].name, variables[i].value);
            DrawTextEx(customFont, varText, (Vector2){15, yOffset+i*18}, 14, 0, ACCENT_PURPLE);
        }

        // Enhanced Canvas
        DrawRectangleRec(canvas,BG_MEDIUM);
        DrawRectangleLinesEx(canvas, 1, BG_LIGHT);
        
        for(int i=0;i<objectCount;i++){
            GameObject *o=&objects[i];
            
            if(o->selected) {
                if(o->shape==SHAPE_CIRCLE) {
                    DrawCircleLinesV(o->position, o->size + 5, ACCENT_BLUE);
                } else {
                    Vector2 topLeft = Vector2Subtract(o->position,(Vector2){o->size+5,o->size+5});
                    DrawRectangleLinesEx((Rectangle){topLeft.x, topLeft.y, (o->size+5)*2, (o->size+5)*2}, 2, ACCENT_BLUE);
                }
            }
            
            if(o->shape==SHAPE_CIRCLE) {
                DrawCircleV(o->position,(int)o->size,o->color);
                DrawCircleLinesV(o->position,(int)o->size,DARKGRAY);
            } else {
                Vector2 topLeft = Vector2Subtract(o->position,(Vector2){o->size,o->size});
                DrawRectangleV(topLeft, (Vector2){o->size*2,o->size*2}, o->color);
                DrawRectangleLinesEx((Rectangle){topLeft.x, topLeft.y, o->size*2, o->size*2}, 1, DARKGRAY);
            }

            float tw = MeasureTextSubstring(o->name,strlen(o->name),12);
            DrawRectangle((int)(o->position.x-tw/2-4),(int)(o->position.y-o->size-20), tw+8, 16, (Color){0,0,0,150});
            DrawTextEx(customFont, o->name, (Vector2){(int)(o->position.x-tw/2), (int)(o->position.y-o->size-18)}, 12, 0, WHITE);
        }

        // Enhanced Bottom command bar
        Rectangle cmdRect={0,(float)(screenH-commandBarH),(float)screenW,(float)commandBarH};
        DrawRectangleRec(cmdRect,BG_DARK);
        DrawRectangle(0, screenH-commandBarH, screenW, 1, BG_LIGHT);
        
        DrawTextEx(customFont, "> ", (Vector2){10, (int)(screenH-commandBarH)+10}, 20, 0, ACCENT_BLUE);
        DrawTextEx(customFont, commandBuffer, (Vector2){35, (int)(screenH-commandBarH)+10}, 20, 0, TEXT_PRIMARY);
        
        static float cursorTimer = 0;
        cursorTimer += GetFrameTime();
        if(cursorTimer < 0.5f) {
            float textWidth = MeasureText(commandBuffer, 20);
            DrawTextEx(customFont, "_", (Vector2){35 + textWidth, (int)(screenH-commandBarH)+10}, 20, 0, ACCENT_BLUE);
        }
        if(cursorTimer > 1.0f) cursorTimer = 0;

        // Enhanced Logs
        DrawRectangle(0, screenH-commandBarH-logBarH, screenW, logBarH, BG_MEDIUM);
        DrawRectangle(0, screenH-commandBarH-logBarH, screenW, 1, BG_LIGHT);
        
        for(int i=0;i<logCount;i++) {
            Color logColor = TEXT_SECONDARY;
            if(strstr(logs[i], "Error:")) logColor = ACCENT_RED;
            else if(strstr(logs[i], "Added") || strstr(logs[i], "Loaded") || strstr(logs[i], "Saved") || strstr(logs[i], "Executed")) logColor = ACCENT_GREEN;
            else if(strstr(logs[i], "[Script]") || strstr(logs[i], "[Python]")) logColor = ACCENT_PURPLE;
            
            DrawTextEx(customFont, logs[i], (Vector2){15, (int)(screenH-commandBarH-logBarH+10+i*20)}, 14, 0, logColor);
        }

        // Enhanced Play button
        DrawRectangleRounded(playButton, 0.2f, 8, running ? ACCENT_GREEN : ACCENT_RED);
        const char* buttonText = running ? "▪ Stop" : "▶ Run";
        DrawTextEx(customFont, buttonText, (Vector2){(int)playButton.x+15, (int)playButton.y+10}, 18, 0, WHITE);

        // Enhanced Script editor with syntax highlighting
        if(activeEditor>=0){
            ScriptEditor *ed=&openEditors[activeEditor];
            int startY=tabBarH; int startX=leftPanelW;
            Rectangle editorRect = {startX, startY, (float)(screenW-leftPanelW), (float)(screenH-bottomH-tabBarH)};
            
            DrawRectangleRec(editorRect, BG_DARK);
            
            // Draw line numbers background
            DrawRectangle(startX, startY, 50, editorRect.height, BG_MEDIUM);
            DrawLine(startX+50, startY, startX+50, startY+editorRect.height, BG_LIGHT);
            
            // Show Script status
            if(ed->isPython) {
                DrawTextEx(customFont, "Python Script - Press F5 to Run", (Vector2){startX + 60, startY + 5}, 12, 0, ACCENT_GREEN);
            } else if(ed->isScript) {
                DrawTextEx(customFont, "Custom Script - Press F5 to Run", (Vector2){startX + 60, startY + 5}, 12, 0, ACCENT_PURPLE);
            }
            
            // Draw editor content with syntax highlighting
            int contentStartY = startY + ((ed->isPython || ed->isScript) ? 25 : 10);
            for(int i=0;i<VISIBLE_LINES && i+ed->scroll < ed->lineCount;i++){
                int lineIdx=i+ed->scroll;
                int lineY = contentStartY + i*editorCharY;
                
                // Line number
                char lineNum[8];
                snprintf(lineNum, sizeof(lineNum), "%d", lineIdx + 1);
                DrawTextEx(customFont, lineNum, (Vector2){startX + 10, lineY}, 14, 0, TEXT_SECONDARY);
                
                // Line content
                if(strlen(ed->lines[lineIdx]) > 0) {
                    if(ed->isPython) {
                                                // Draw the whole line first
                        DrawTextEx(customFont, ed->lines[lineIdx], (Vector2){startX + 60, lineY}, 14, 0, TEXT_PRIMARY);
                        
                        // Python keywords
                        const char* keywords[] = {
                            "and", "assert", "break", "class", "continue", "def",
                            "del", "elif", "else", "except", "exec", "finally",
                            "for", "from", "global", "if", "import", "in",
                            "is", "lambda", "not", "or", "pass", "print",
                            "raise", "return", "try", "while", "yield",
                            "None", "True", "False"
                        };
                        int keywordCount = sizeof(keywords) / sizeof(keywords[0]);
                        
                        // Built-in functions
                        const char* builtins[] = {"len", "range", "str", "int", "float", "list", "dict", "open", "abs", "min", "max"};
                        int builtinCount = sizeof(builtins) / sizeof(builtins[0]);
                        
                        // Game API functions
                        const char* gameAPI[] = {"move_object", "set_position", "set_color", "create_circle", "create_rect", "log_message"};
                        int gameAPICount = sizeof(gameAPI) / sizeof(gameAPI[0]);
                        
                        char *line = ed->lines[lineIdx];
                        
                        // Highlight keywords
                        for(int k = 0; k < keywordCount; k++) {
                            char *pos = strstr(line, keywords[k]);
                            while(pos != NULL) {
                                // Make sure it's a whole word (not part of another word)
                                bool isWholeWord = (pos == line || !isalnum(*(pos-1))) && 
                                                  (!isalnum(pos[strlen(keywords[k])]));
                                
                                if(isWholeWord) {
                                    int charOffset = pos - line;
                                    float xOffset = MeasureTextEx(customFont, line, 14, 0).x * charOffset / strlen(line);
                                    DrawTextEx(customFont, keywords[k], (Vector2){startX + 60 + xOffset, lineY}, 14, 0, ACCENT_BLUE);
                                }
                                pos = strstr(pos + 1, keywords[k]);
                            }
                        }
                        
                        // Highlight built-ins
                        for(int b = 0; b < builtinCount; b++) {
                            char *pos = strstr(line, builtins[b]);
                            while(pos != NULL) {
                                bool isWholeWord = (pos == line || !isalnum(*(pos-1))) && 
                                                  (!isalnum(pos[strlen(builtins[b])]));
                                
                                if(isWholeWord) {
                                    int charOffset = pos - line;
                                    float xOffset = MeasureTextEx(customFont, line, 14, 0).x * charOffset / strlen(line);
                                    DrawTextEx(customFont, builtins[b], (Vector2){startX + 60 + xOffset, lineY}, 14, 0, ACCENT_PURPLE);
                                }
                                pos = strstr(pos + 1, builtins[b]);
                            }
                        }
                        
                        // Highlight game API
                        for(int g = 0; g < gameAPICount; g++) {
                            char *pos = strstr(line, gameAPI[g]);
                            while(pos != NULL) {
                                bool isWholeWord = (pos == line || !isalnum(*(pos-1))) && 
                                                  (!isalnum(pos[strlen(gameAPI[g])]));
                                
                                if(isWholeWord) {
                                    int charOffset = pos - line;
                                    float xOffset = MeasureTextEx(customFont, line, 14, 0).x * charOffset / strlen(line);
                                    DrawTextEx(customFont, gameAPI[g], (Vector2){startX + 60 + xOffset, lineY}, 14, 0, ACCENT_GREEN);
                                }
                                pos = strstr(pos + 1, gameAPI[g]);
                            }
                        }
                                                // Highlight numbers (yellow/orange)
                        char *linePtr = line;
                        while(*linePtr) {
                            if(isdigit(*linePtr) || (*linePtr == '.' && isdigit(*(linePtr+1)))) {
                                // Found start of a number
                                char *numStart = linePtr;
                                
                                // Find end of number (including decimals)
                                while(isdigit(*linePtr) || *linePtr == '.') {
                                    linePtr++;
                                }
                                
                                // Extract the number
                                int numLen = linePtr - numStart;
                                char numberStr[32];
                                strncpy(numberStr, numStart, numLen);
                                numberStr[numLen] = '\0';
                                
                                // Calculate position and draw
                                int charOffset = numStart - line;
                                float xOffset = MeasureTextEx(customFont, line, 14, 0).x * charOffset / strlen(line);
                                DrawTextEx(customFont, numberStr, (Vector2){startX + 60 + xOffset, lineY}, 14, 0, ACCENT_ORANGE);
                            } else {
                                linePtr++;
                            }
                        }

                        // Highlight operators (green)
                        const char* operators[] = {
                            "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", 
                            "//", "**", "<<", ">>", "=", "<", ">", "+", "-", "*", 
                            "/", "%", "&", "|", "^", "~"
                        };
                        int operatorCount = sizeof(operators) / sizeof(operators[0]);

                        for(int o = 0; o < operatorCount; o++) {
                            char *pos = strstr(line, operators[o]);
                            while(pos != NULL) {
                                // Make sure it's not part of a larger operator
                                bool validOp = true;
                                if(strlen(operators[o]) == 1) {
                                    // For single-char operators, check they're not part of multi-char ones
                                    char prev = (pos > line) ? *(pos-1) : ' ';
                                    char next = *(pos+1);
                                    if((prev == '=' || prev == '!' || prev == '<' || prev == '>') ||
                                       (next == '=' || (operators[o][0] == '*' && next == '*'))) {
                                        validOp = false;
                                    }
                                }
                                
                                if(validOp) {
                                    int charOffset = pos - line;
                                    float xOffset = MeasureTextEx(customFont, line, 14, 0).x * charOffset / strlen(line);
                                    DrawTextEx(customFont, operators[o], (Vector2){startX + 60 + xOffset, lineY}, 14, 0, ACCENT_GREEN);
                                }
                                pos = strstr(pos + 1, operators[o]);
                            }
                        }
                    } else if(ed->isScript) {
                        // Custom script syntax highlighting
                        char *line = ed->lines[lineIdx];
                        int x = startX + 60;
                        char word[64] = {0};
                        int wordIdx = 0;
                        
                        for(int c = 0; line[c]; c++) {
                            if(isalnum(line[c]) || line[c] == '_') {
                                word[wordIdx++] = line[c];
                            } else {
                                if(wordIdx > 0) {
                                    word[wordIdx] = '\0';
                                    Color tokenColor = GetScriptTokenColor(word);
                                    DrawTextEx(customFont, word, (Vector2){x, lineY}, 14, 0, tokenColor);
                                    x += MeasureTextEx(customFont, word, 14, 0).x;
                                    wordIdx = 0;
                                }
                                char charStr[2] = {line[c], '\0'};
                                Color charColor = TEXT_PRIMARY;
                                if(line[c] == '=' || line[c] == '+' || line[c] == '-' || line[c] == '*' || line[c] == '/') {
                                    charColor = ACCENT_ORANGE;
                                } else if(line[c] == '"') {
                                    charColor = ACCENT_GREEN;
                                }
                                DrawTextEx(customFont, charStr, (Vector2){x, lineY}, 14, 0, charColor);
                                x += MeasureTextEx(customFont, charStr, 14, 0).x;
                            }
                        }
                        
                        if(wordIdx > 0) {
                            word[wordIdx] = '\0';
                            Color tokenColor = GetScriptTokenColor(word);
                            DrawTextEx(customFont, word, (Vector2){x, lineY}, 14, 0, tokenColor);
                        }
                    } else {
                        DrawTextEx(customFont, ed->lines[lineIdx], (Vector2){startX + 60, lineY}, 14, 0, TEXT_PRIMARY);
                    }
                }
            }
            
            // Enhanced Caret
            if(caretLine >= ed->scroll && caretLine < ed->scroll + VISIBLE_LINES) {
                int caretX = startX + 60 + (int)MeasureTextSubstring(ed->lines[caretLine], caretCol, 14);
                int caretY = contentStartY + (caretLine - ed->scroll) * editorCharY;
                
                static float caretBlink = 0;
                caretBlink += GetFrameTime();
                if(caretBlink < 0.5f) {
                    DrawLine(caretX, caretY, caretX, caretY + editorCharY - 2, ACCENT_BLUE);
                }
                if(caretBlink > 1.0f) caretBlink = 0;
            }
            
            // Current line highlight
            if(caretLine >= ed->scroll && caretLine < ed->scroll + VISIBLE_LINES) {
                int highlightY = contentStartY + (caretLine - ed->scroll) * editorCharY;
                DrawRectangle(startX + 50, highlightY - 2, screenW - leftPanelW - 50, editorCharY, (Color){ACCENT_BLUE.r, ACCENT_BLUE.g, ACCENT_BLUE.b, 20});
            }
        }

        // Help text
        DrawTextEx(customFont, "Commands: NewScript/NewPython <n>, RunScript/RunPython <n>, set var value", (Vector2){10, screenH-15}, 10, 0, TEXT_SECONDARY);

        EndDrawing();
    }

    Py_Finalize();
    CloseWindow();
    return 0;
}
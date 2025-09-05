#include "../include/engine_io.h"
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <windows.h>
#include <stdlib.h>

void CreateDirIfNotExist(const char* path){
    // _mkdir returns -1 on error, but we don't care if it already exists
    _mkdir(path);
}

void UpdateProjectIO(const char* name, const ObjectIO* objects, int objectCount){
    char path[256]; snprintf(path,sizeof(path),"./Projects/%s",name);
    CreateDirIfNotExist("./Projects"); CreateDirIfNotExist(path);
    char mainFile[256]; snprintf(mainFile,sizeof(mainFile),"%s/main.game",path);
    FILE *f = fopen(mainFile,"w");
    if(f){ 
        fprintf(f,"# %s Project\n",name);
        for(int i=0;i<objectCount;i++){
            const ObjectIO* o = &objects[i];
            fprintf(f,"%s %s %.0f %.0f %.0f %d %d %d\n",
                o->shape==0?"circle":"rect", o->name, o->x,o->y,o->size,
                o->r,o->g,o->b);
        }
        fclose(f);
    }
    char scriptsDir[256]; snprintf(scriptsDir,sizeof(scriptsDir),"%s/scripts",path);
    CreateDirIfNotExist(scriptsDir);
    AddLog("Project '%s' updated", name);
}

int LoadScriptsIO(const char* projectName, Script* outScripts, int maxScripts){
    int scriptCount = 0;
    char path[256]; snprintf(path,sizeof(path),"./Projects/%s/scripts",projectName);

    // Build search pattern
    char pattern[512]; snprintf(pattern,sizeof(pattern),"%s/*", path);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if(h == INVALID_HANDLE_VALUE){ AddLog("No scripts folder or none found: %s", path); return 0; }
    do {
        if(strcmp(fd.cFileName, ".")==0 || strcmp(fd.cFileName, "..")==0) continue;
        if(scriptCount >= maxScripts) break;
        Script *s = &outScripts[scriptCount]; s->lineCount=0;
        strncpy(s->name, fd.cFileName, NAME_LEN-1);
        s->name[NAME_LEN-1]=0;
        s->isScript = (strstr(fd.cFileName, ".script") != NULL);
        s->isPython = (strstr(fd.cFileName, ".py") != NULL);
        char filePath[512]; snprintf(filePath,sizeof(filePath),"%s/%s", path, fd.cFileName);
        FILE *f=fopen(filePath,"r"); if(!f) continue;
        while(fgets(s->lines[s->lineCount], LINE_LEN, f) && s->lineCount<128){
            char *p=strchr(s->lines[s->lineCount],'\n'); if(p)*p=0;
            s->lineCount++;
        }
        fclose(f);
        const char* typeStr = s->isPython ? "Python script" : (s->isScript ? "custom script" : "file");
        AddLog("Loaded %s '%s'", typeStr, s->name);
        scriptCount++;
    } while(FindNextFileA(h, &fd));
    FindClose(h);
    return scriptCount;
}

int SaveScriptToProject(const char* projectName, const char* filename, char lines[][LINE_LEN], int lineCount){
    char scriptsDir[256];
    snprintf(scriptsDir, sizeof(scriptsDir), "./Projects/%s/scripts", projectName);
    CreateDirIfNotExist("./Projects");
    CreateDirIfNotExist(scriptsDir);

    char path[512]; snprintf(path, sizeof(path), "%s/%s", scriptsDir, filename);
    FILE *f = fopen(path,"w");
    if(!f){ AddLog("Failed to save '%s'", filename); return 0; }
    for(int i=0;i<lineCount;i++) fprintf(f, "%s\n", lines[i]);
    fclose(f);
    AddLog("Saved '%s' in project '%s'", filename, projectName);
    return 1;
}

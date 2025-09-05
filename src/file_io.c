#define RAYLIB_NO_CONFLICT_WINDOWS
#include <raylib.h>
#include <stdio.h>
#include <direct.h>
#include <sys/stat.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "../include/engine_io.h"

void CreateDirIfNotExist(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        #ifdef _WIN32
            _mkdir(path);
        #else
            mkdir(path, 0700);
        #endif
    }
}

void LoadScripts(const char* projectName) {
    char searchPath[256];
    snprintf(searchPath, sizeof(searchPath), "./Projects/%s/scripts/*.py", projectName);
    
    #ifdef _WIN32
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(searchPath, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                char name[NAME_LEN];
                strncpy(name, fd.cFileName, NAME_LEN-1);
                name[NAME_LEN-1] = '\0';
                
                if (strstr(fd.cFileName, ".py") || strstr(fd.cFileName, ".PY")) {
                    char fullPath[256];
                    snprintf(fullPath, sizeof(fullPath), "./Projects/%s/scripts/%s", projectName, fd.cFileName);
                    // TODO: Load script content
                }
            }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    #endif
}

void SaveScripts(const char* projectName) {
    // TODO: Implement script saving
}

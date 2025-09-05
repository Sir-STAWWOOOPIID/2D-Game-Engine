#define RAYLIB_NO_CONFLICT_WINDOWS
#include <raylib.h>
#include "../include/core.h"

#include <stdio.h>
#include <stdarg.h>
#include "../include/engine_io.h"

// Remove duplicate color definitions; use extern from core.c

void AddLog(const char* fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("[LOG] %s\n", buf);
}

void ExecuteCommand(const char* cmd)
{
    if (!cmd) return;
    printf("[EXEC_CMD] %s\n", cmd);
}

void AddObject(const char* name, int shape, Vector2 pos, int size, Color color)
{
    (void)shape; // stub ignores shape
    printf("[AddObject] name='%s' pos=(%.1f,%.1f) size=%d color=(%d,%d,%d)\n",
           name ? name : "(null)", pos.x, pos.y, size, color.r, color.g, color.b);
}

void SetVariable(const char* name, float value)
{
    printf("[SetVariable] %s = %f\n", name ? name : "(null)", value);
}

void LoadScripts(const char* projectName)
{
    printf("[LoadScripts] project='%s'\n", projectName ? projectName : "(null)");
}

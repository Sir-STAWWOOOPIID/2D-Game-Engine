#ifndef CORE_H
#define CORE_H

#include <raylib.h>
#include <stdbool.h>

#define MAX_OBJECTS 128
#define NAME_LEN    32
#define LOG_CAP     3
#define CMD_LEN     256
#define MAX_SCRIPTS 64
#define LINE_LEN    128
#define MAX_OPEN_EDITORS 16
#define VISIBLE_LINES 20
#define MAX_VARIABLES 64

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
    bool isScript;
    bool isPython;
} Script;

typedef struct {
    char name[NAME_LEN];
    char lines[128][LINE_LEN];
    int lineCount;
    int scroll;
    bool isScript;
    bool isPython;
} ScriptEditor;

// Color scheme
extern const Color BG_DARK;
extern const Color BG_MEDIUM;
extern const Color BG_LIGHT;
extern const Color ACCENT_BLUE;
extern const Color ACCENT_GREEN;
extern const Color ACCENT_RED;
extern const Color ACCENT_ORANGE;
extern const Color ACCENT_PURPLE;
extern const Color TEXT_PRIMARY;
extern const Color TEXT_SECONDARY;

#endif // CORE_H

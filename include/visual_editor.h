#ifndef VISUAL_EDITOR_H
#define VISUAL_EDITOR_H

#include "core.h"

// Object manipulation and selection
GameObject* FindByName(const char* name);
bool NameExists(const char* name);
void AddObject(const char* name, ShapeType shape, Vector2 pos, float size, Color col);
void DeleteGameObject(const char* name);
void ClearSelection(void);
bool PointInObject(Vector2 pt, GameObject* obj);

// Dragging logic
extern bool dragging;
extern Vector2 dragOffset;

#endif // VISUAL_EDITOR_H

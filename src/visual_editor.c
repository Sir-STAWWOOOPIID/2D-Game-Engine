#include "../include/visual_editor.h"
#include "../include/core.h"
#include "../include/ui.h"
#include <string.h>
#include <raylib.h>

GameObject objects[MAX_OBJECTS];
int objectCount = 0;
bool dragging = false;
Vector2 dragOffset = {0};

GameObject* FindByName(const char* name) {
    for(int i=0;i<objectCount;i++)
        if(strcmp(objects[i].name,name)==0)
            return &objects[i];
    return NULL;
}

bool NameExists(const char* name) {
    return FindByName(name)!=NULL;
}

void AddObject(const char* name, ShapeType shape, Vector2 pos, float size, Color col) {
    if(objectCount >= MAX_OBJECTS){ AddLog("Error: object limit reached"); return; }
    if(NameExists(name)){ AddLog("Error: name '%s' exists", name); return; }
    GameObject o = {0}; strncpy(o.name, name, NAME_LEN-1); o.position = pos; o.size = (size <= 0 ? 1 : size); o.color = col; o.shape = shape; o.selected = false;
    objects[objectCount++] = o; AddLog("Added %s '%s'", shape==SHAPE_CIRCLE?"circle":"rect", name);
}

void DeleteGameObject(const char* name) {
    for(int i=0;i<objectCount;i++){
        if(strcmp(objects[i].name,name)==0){
            for(int j=i+1;j<objectCount;j++) objects[j-1]=objects[j];
            objectCount--;
            AddLog("Deleted '%s'",name);
            return;
        }
    }
    AddLog("Error: '%s' not found",name);
}

void ClearSelection(void) {
    for(int i=0;i<objectCount;i++) objects[i].selected = false;
}

bool PointInObject(Vector2 pt, GameObject* obj) {
    if(obj->shape == SHAPE_CIRCLE) {
        float dx = pt.x - obj->position.x;
        float dy = pt.y - obj->position.y;
        return (dx*dx + dy*dy) <= (obj->size * obj->size);
    } else {
        Vector2 topLeft = {obj->position.x - obj->size, obj->position.y - obj->size};
        return pt.x >= topLeft.x && pt.x <= topLeft.x + obj->size*2 && pt.y >= topLeft.y && pt.y <= topLeft.y + obj->size*2;
    }
}

int GetObjectCount(void) { return objectCount; }
GameObject* GetObjects(void) { return objects; }

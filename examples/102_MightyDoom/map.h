// map.h
#pragma once

#include "libs.h"

typedef struct {
    T3DModel *model;
    T3DMat4FP *matrix;
} Map;

void map_init(Map *map);
void map_draw(Map *map);
void map_destroy(Map *map);
// map.c
#include "map.h"

void map_init(Map *map) {
    map->model = t3d_model_load("rom:/map.t3dm");
    map->matrix = malloc_uncached(sizeof(T3DMat4FP));

    t3d_mat4fp_from_srt_euler(map->matrix,
        (float[3]){0.3f, 0.3f, 0.3f},   // scale
        (float[3]){0, 0, 0},           // rotation
        (float[3]){0, 0, -10}          // position
    );
}

void map_draw(Map *map) {
    t3d_matrix_push(map->matrix);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(map->model);
    t3d_matrix_pop(1);
}

void map_destroy(Map *map) {
    t3d_model_free(map->model);
    // Optionally free map->matrix if you want to release memory
}

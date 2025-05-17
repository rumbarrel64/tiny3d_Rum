// zombie.h
#pragma once

#include "libs.h"

typedef struct {
    T3DVec3 position;
    float rotation_y;

    T3DModel *model;
    T3DModel *model_shadow;

    T3DSkeleton skel;
    T3DAnim anim_walk;

    T3DMat4FP *model_matrix;
    float speed;

    int health;
    bool alive;
} Zombie;

// Initialize one zombie with a starting position
void zombie_init(Zombie *z, const T3DVec3 *start_pos);

// Update one zombie's movement and animation
//void zombie_update(Zombie *z, const T3DVec3 *player_pos, float delta_time);
void zombie_update(Zombie *z, const T3DVec3 *player_pos, float delta_time, Zombie *zombies, int count);

// Draw one zombie
void zombie_draw(Zombie *z);

// Clean up a single zombie's resources
void zombie_destroy(Zombie *z);

// Bulk helpers
void zombie_update_all(Zombie *zombies, int count, const T3DVec3 *player_pos, float delta_time);
void zombie_draw_all(Zombie *zombies, int count);
void zombie_destroy_all(Zombie *zombies, int count);

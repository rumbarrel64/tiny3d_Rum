#pragma once

#include "libs.h"
#include "slayer.h"

typedef struct {
    T3DVec3 position;
    T3DVec3 direction;
    bool active;
} Bullet;

typedef struct {
    Bullet *bullets;        // Array of bullet data
    uint32_t max_count;
    float speed;
    float size_x;
    float size_y;
    TPXParticle *buffer;
    T3DMat4FP *matrices;    // One matrix per bullet
} BulletSystem;

void bullet_init(BulletSystem *bs, uint32_t max_particles, float size_x, float size_y);
void bullet_update(BulletSystem *bs, const T3DVec3 *origin, float rot_y, float box_limit, Zombie *zombies, int *zombie_count, joypad_buttons_t btn);
void bullet_draw(BulletSystem *bs);
void bullet_cleanup(BulletSystem *bs);

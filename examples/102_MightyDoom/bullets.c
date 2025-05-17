#include "bullets.h"
#include <float.h>
#include <malloc.h>

void bullet_init(BulletSystem *bs, uint32_t max_bullets, float size_x, float size_y) {
    bs->max_count = max_bullets;
    bs->size_x = size_x;
    bs->size_y = size_y;
    bs->speed = 1.5f; // Bullet speed

    bs->bullets = malloc_uncached(sizeof(Bullet) * max_bullets);
    bs->buffer = malloc_uncached(sizeof(TPXParticle) * max_bullets);
    bs->matrices = malloc_uncached(sizeof(T3DMat4FP) * max_bullets);

    for (uint32_t i = 0; i < max_bullets; i++) {
        bs->bullets[i].active = false;

        // Init particle visual
        int8_t *ptPos = bs->buffer[i].posA;
        uint8_t *ptColor = bs->buffer[i].colorA;
        bs->buffer[i].sizeA = 50;
        ptPos[0] = 1;
        ptPos[1] = 20;
        ptPos[2] = 0;
        ptColor[0] = 0;
        ptColor[1] = 0;
        ptColor[2] = 255;
        ptColor[3] = 255;
    }
}

void bullet_update(BulletSystem *bs, const T3DVec3 *origin, float rot_y, float box_limit, Zombie *zombies, int *zombie_count, joypad_buttons_t btn) {
    const float ZOMBIE_HIT_RADIUS2 = 50.0f;

    // 🔫 Spawn bullet if B is pressed
    if (btn.b) {
        for (uint32_t i = 0; i < bs->max_count; i++) {
            if (!bs->bullets[i].active) {
                bs->bullets[i].position = *origin;
                bs->bullets[i].direction.v[0] = sinf(rot_y);
                bs->bullets[i].direction.v[1] = 0;
                bs->bullets[i].direction.v[2] = cosf(rot_y);
                bs->bullets[i].active = true;
                break; // fire one bullet per press
            }
        }
    }

    for (uint32_t i = 0; i < bs->max_count; i++) {
        if (!bs->bullets[i].active) continue;

        Bullet *b = &bs->bullets[i];

        // Move bullet
        b->position.v[0] += b->direction.v[0] * bs->speed;
        b->position.v[2] += b->direction.v[2] * bs->speed;

        // Out of bounds check
        if (b->position.v[0] < -box_limit || b->position.v[0] > box_limit ||
            b->position.v[2] < -box_limit || b->position.v[2] > box_limit) {
            b->active = false;
            continue;
        }

        // Check zombie collisions
        for (int j = 0; j < *zombie_count; j++) {
            if (!zombies[j].alive) continue;

            float dx = b->position.v[0] - zombies[j].position.v[0];
            float dz = b->position.v[2] - zombies[j].position.v[2];
            float dist2 = dx * dx + dz * dz;

            if (dist2 <= ZOMBIE_HIT_RADIUS2) {
                zombies[j].health--;
                if (zombies[j].health <= 0) {
                    zombies[j].alive = false;
                }
                b->active = false;
                break;
            }
        }

        // Update matrix for this bullet
        T3DVec3 scale = {{0.8f, 0.8f, 0.8f}};
        T3DVec3 rotation = {{0.0f, 0.0f, 0.0f}};
        t3d_mat4fp_from_srt_euler(&bs->matrices[i], scale.v, rotation.v, b->position.v);
    }
}

void bullet_draw(BulletSystem *bs) {
    rdpq_set_env_color((color_t){0xFF, 0xFF, 0xFF, 0xFF});
    rdpq_sync_pipe();
    rdpq_sync_tile();
    rdpq_set_mode_standard();
    rdpq_mode_zbuf(true, true);
    rdpq_mode_zoverride(true, 0, 0);
    rdpq_mode_combiner(RDPQ_COMBINER1((PRIM, 0, ENV, 0), (0, 0, 0, 1)));

    tpx_state_from_t3d();
    tpx_state_set_scale(bs->size_x, bs->size_y);

    for (uint32_t i = 0; i < bs->max_count; i++) {
        if (!bs->bullets[i].active) continue;
        tpx_matrix_push(&bs->matrices[i]);
        tpx_particle_draw(&bs->buffer[i], 2); // 2 for two-particle mode
        tpx_matrix_pop(1);
    }
}

void bullet_cleanup(BulletSystem *bs) {
    free(bs->bullets);
    free(bs->buffer);
    free(bs->matrices);
}

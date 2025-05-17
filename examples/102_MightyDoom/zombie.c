// zombie.c
#include "zombie.h"
#include "collision.h"

void zombie_init(Zombie *z, const T3DVec3 *start_pos) {
    z->position = *start_pos;
    z->rotation_y = 0.0f;
    z->speed = 0.3f;

    z->model = t3d_model_load("rom:/slayer.t3dm");
    z->model_shadow = t3d_model_load("rom:/shadow.t3dm");
    z->model_matrix = malloc_uncached(sizeof(T3DMat4FP));

    z->skel = t3d_skeleton_create(z->model);
    z->anim_walk = t3d_anim_create(z->model, "walking-left");
    t3d_anim_attach(&z->anim_walk, &z->skel);

    z->health = 5;
    z->alive = true;
}

//void zombie_update(Zombie *z, const T3DVec3 *player_pos, float delta_time) {
void zombie_update(Zombie *z, const T3DVec3 *player_pos, float delta_time, Zombie *zombies, int count) {
/* old
    float dx = player_pos->v[0] - z->position.v[0];
    float dz = player_pos->v[2] - z->position.v[2];

    float dist = sqrtf(dx * dx + dz * dz);
    if (dist > 1.0f) {
        dx /= dist;
        dz /= dist;
        z->position.v[0] += dx * z->speed;
        z->position.v[2] += dz * z->speed;
        z->rotation_y = atan2f(dx, dz);
    }

    t3d_anim_update(&z->anim_walk, delta_time);
    t3d_skeleton_update(&z->skel);

    t3d_mat4fp_from_srt_euler(z->model_matrix,
        (float[3]){0.0035f, 0.0035f, 0.0035f},
        (float[3]){-4.5f, 0.0f, -z->rotation_y},
        z->position.v
    );
    */

    float dx = player_pos->v[0] - z->position.v[0];
    float dz = player_pos->v[2] - z->position.v[2];
    
    float dist = sqrtf(dx * dx + dz * dz);

    if (dist > 1.0f) {
        dx /= dist;
        dz /= dist;

        // Simulate new position
        T3DVec3 next_pos = z->position;
        next_pos.v[0] += dx * z->speed;
        next_pos.v[2] += dz * z->speed;

        // Check collision with player
        if (check_overlap(&next_pos, player_pos, 5.0f)) return;

        // Check collision with other zombies
        for (int i = 0; i < count; i++) {
            Zombie *other = &zombies[i];
            if (other == z || other->health <= 0) continue;
            if (check_overlap(&next_pos, &other->position, 5.0f)) return;
        }

        // Apply movement only if not blocked
        z->position = next_pos;
        z->rotation_y = atan2f(dx, dz);
    }

    t3d_anim_update(&z->anim_walk, delta_time);
    t3d_skeleton_update(&z->skel);

    t3d_mat4fp_from_srt_euler(z->model_matrix,
        (float[3]){0.0035f, 0.0035f, 0.0035f},
        (float[3]){-4.5f, 0.0f, -z->rotation_y},
        z->position.v
    );
}

void zombie_draw(Zombie *z) {
    t3d_matrix_push(z->model_matrix);
    rdpq_set_prim_color(RGBA32(255, 0, 0, 255));
    t3d_model_draw_skinned(z->model, &z->skel);

    rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
    t3d_model_draw(z->model_shadow);
    t3d_matrix_pop(1);
}

void zombie_destroy(Zombie *z) {
    t3d_model_free(z->model);
    t3d_model_free(z->model_shadow);
    t3d_skeleton_destroy(&z->skel);
    t3d_anim_destroy(&z->anim_walk);
    // optionally: free(z->model_matrix);
}

// ----- Multiple Zombie Helpers -----

void zombie_update_all(Zombie *zombies, int count, const T3DVec3 *player_pos, float delta_time) {
    for (int i = 0; i < count; i++) {
        if (!zombies[i].alive) continue;
        zombie_update(&zombies[i], player_pos, delta_time, zombies, count);
    }
}
/*
void zombie_update_all(Zombie *zombies, int count, const T3DVec3 *player_pos, float delta_time) {
    for (int i = 0; i < count; i++) {
        if (!zombies[i].alive) continue;
        zombie_update(&zombies[i], player_pos, delta_time);
    }
}
*/

void zombie_draw_all(Zombie *zombies, int count) {
    for (int i = 0; i < count; i++) {
        zombie_draw(&zombies[i]);
    }
}

void zombie_destroy_all(Zombie *zombies, int count) {
    for (int i = 0; i < count; i++) {
        zombie_destroy(&zombies[i]);
    }
    free(zombies);
}

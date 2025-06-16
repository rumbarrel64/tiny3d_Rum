// zombie.c
#include "zombie.h"
#include "collision.h"

void zombie_init(Zombie *z, const T3DVec3 *start_pos) {
    
    // Zombie Positions
    z->position = *start_pos;
    z->rotation_y = 0.0f;
    z->speed = 0.3f;

    // Zombie Health
    z->health = 5;
    z->alive = true;

    // Zombie Death 
    z ->blood_time = 0;
    z ->blood_scale = 0.3f;

    //z->model = t3d_model_load("rom:/slayer.t3dm"); 
    z->model = t3d_model_load("rom:/zombieWalking.t3dm"); 
    z->model_matrix = malloc_uncached(sizeof(T3DMat4FP));

    // Zombie Skeletons for model
    z->skel = t3d_skeleton_create(z->model); // Comment out for new model
    
    // Zombie Animations
    z->anim_walk = t3d_anim_create(z->model, "walking-left"); // Comment out for new model
    t3d_anim_attach(&z->anim_walk, &z->skel);  // Comment out for new model
}

void zombie_update(Zombie *z, const T3DVec3 *player_pos, float delta_time, Zombie *zombies, int count) {

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
        if (check_overlap(&next_pos, player_pos, ZOMBIE_COLLISION_RADIUS)) return;

        // Check collision with other zombies
        for (int i = 0; i < count; i++) {
            Zombie *other = &zombies[i];
            if (other == z || other->health <= 0) continue;
            if (check_overlap(&next_pos, &other->position, ZOMBIE_COLLISION_RADIUS)) return;
        }

        // Apply movement only if not blocked
        z->position = next_pos;
        z->rotation_y = atan2f(dx, dz);
    }

    t3d_anim_update(&z->anim_walk, delta_time);// Comment out for new model
    t3d_skeleton_update(&z->skel); // Comment out for new model

    t3d_mat4fp_from_srt_euler(z->model_matrix,
        // Scale
        (float[3]){0.0035f, 0.0035f, 0.0035f}, // Comment out for new model
        //(float[3]){1.0f, 1.0f, 1.0f},// New model

        // Rotation
        (float[3]){-4.5f, 0.0f, -z->rotation_y},
        // Position
        z->position.v
    );
}

void draw_zombie_health_bar(const Zombie *z, T3DViewport *viewport) {
    if (!z->alive || z->health <= 0) return;

    // Offset above the zombie's head
    T3DVec3 world_pos = z->position;
    world_pos.v[1] += 60.0f; // raise it above the head (50 is good if camera is behind player)

    T3DVec3 screen_pos;
    t3d_viewport_calc_viewspace_pos(viewport, &screen_pos, &world_pos);

    const float width = 30.0f;
    const float height = 4.0f;
    float health_pct = (float)z->health / 5;//ZOMBIE_MAX_HEALTH;

    float x0 = screen_pos.v[0] - width / 2.0f;
    float y0 = screen_pos.v[1];

    // Background bar
    rdpq_set_mode_fill(RGBA32(50, 50, 50, 255)); // background
    rdpq_fill_rectangle(x0, y0, x0 + width, y0 + height);

    // Health Bar
    rdpq_set_mode_fill(RGBA32(255 * (1.0f - health_pct), 255 * health_pct, 0, 255));
    rdpq_fill_rectangle(x0, y0, x0 + width * health_pct, y0 + height);

    // Damage numbers (needs more work)
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, x0 + width / 2.0f - 4, y0 - 10, "%d", 1);
}

void zombie_draw(Zombie *z) {
    t3d_matrix_push(z->model_matrix);
    rdpq_set_prim_color(RGBA32(255, 0, 0, 255));
    t3d_model_draw_skinned(z->model, &z->skel); // Comment out for new model
    t3d_matrix_pop(1);
}

void zombie_destroy(Zombie *z) {
    // Destroy animations first (they may reference skeletons)
    t3d_anim_destroy(&z->anim_walk); // Comment out for new m
    // Then destroy skeletons
    t3d_skeleton_destroy(&z->skel); // Comment out for new m
    // Free model matrix
    free_uncached(z->model_matrix);
    // Free model
    t3d_model_free(z->model);
}

// ----- Multiple Zombie Helpers -----

void zombie_update_all(Zombie *zombies, int count, const T3DVec3 *player_pos, float delta_time) {
    for (int i = 0; i < count; i++) {
        if (!zombies[i].alive) continue;
        zombie_update(&zombies[i], player_pos, delta_time, zombies, count);
    }
}

void zombie_draw_all(Zombie *zombies, int count) {
    for (int i = 0; i < count; i++) {
        if (!zombies[i].alive) continue;
        zombie_draw(&zombies[i]);
    }
}

void zombie_destroy_all(Zombie *zombies, int count) {
    for (int i = 0; i < count; i++) {
        zombie_destroy(&zombies[i]);
    }
    free_uncached(zombies);  // Only do this if you're done using it
}

// tutorial.c
// General file to hold all libraries
#include "libs.h"

// Custom game libraries
#include "slayer.h"
#include "bullets.h"
#include "map.h"
#include "camera.h"
#include "zombie.h"
#include "levels.h"
#include "level_update.h"
#include "music.h"
#include "banners.h"
#include "gameState.h"
// for debugging
#include <stdio.h>


// Needed for syncing?????
float get_time_s() {
  return (float)((double)get_ticks_us() / 1000000.0);
}

// Tracking time per level This is broken now becuase of menu
float level_start_time = 0.0f;

void tutorial_loop() {

  T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
  t3d_vec3_norm(&lightDirVec);
  uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
  uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

  // Load in Music
  music_load("Tutorial_5_5_11_5.wav64");

   // Level definition
    const LevelData *levels[] = {
        &LEVEL_1,
        &LEVEL_2
    };
    int current_level_index = 0;
    const LevelData *level = levels[current_level_index];

  // Initialize Camera
  Camera camera;
  camera_init(&camera);
  // Initialize Camera

  
  // Initialize Bullet
  Bullet bullet;
  bullet_init(&bullet);
  // Initialize Bullet
  
  // Initialize Map(s)
  Map map, mapWall, mapPortal;
  map_init(&map, "rom:/map.t3dm");
  map_init(&mapWall, "rom:/mapWall.t3dm");
  map_init(&mapPortal, "rom:/mapPortal.t3dm");
  // Initialize Map(s)

  // Initialize Player
  Player player;
  player_init(&player);
  player.position = level->player.position;
  player.rotation_y = level->player.rotation_y;
  // Initialize Player

  // Initialize Zombie(s)
  //int zombie_count = 0;
  //Zombie *zombies = malloc_uncached(sizeof(Zombie) * zombie_count);
  int zombie_count = level->zombie_count;
  Zombie *zombies = malloc_uncached(sizeof(Zombie) * zombie_count);
  for (int i = 0; i < zombie_count; i++) {
      zombie_init(&zombies[i], &level->zombies[i].position);
      zombies[i].rotation_y = level->zombies[i].rotation_y;
  }
  // Initialize Zombie(s)

  // Initilaize Enemy count (Sum off all enemy counts)
  int enemy_count = zombie_count;
  //int enemy_count = 0; // Only use for debugging
  // Initilaize Enemy count

  // Initialize Banners
  banners_init();
  // Allocate array of T3DMat4FP based on enemy count
  T3DMat4FP* spawn_matrices = malloc(sizeof(T3DMat4FP) * enemy_count);
  T3DMat4FP* blood_matrices = malloc(sizeof(T3DMat4FP) * enemy_count);
  // Initialize Banners

  // Initialize Sprite(s)
  //sprite_t *weapon_circle = sprite_load("rom:/weapon_circle.rgba32.sprite");
  //sprite_t *arrow = sprite_load("rom:/arrow.rgba32.sprite");
  // Initialize Sprite(s)

  float lastTime = get_time_s() - (1.0f / 60.0f);
  rspq_syncpoint_t syncPoint = 0;

    while (state ==  STATE_TUTORIAL) {

      // ======== Update ======== //

      // get memory usage (Must be inside the loop to get memory per frame)
      heap_stats_t heap_stats;
      sys_get_heap_stats(&heap_stats);

      //The joypad subsystem only polls the controllers once per VI interrupt.
      joypad_poll();

      float newTime = get_time_s();
      float deltaTime = newTime - lastTime;
      lastTime = newTime;

      joypad_inputs_t joypad = joypad_get_inputs(JOYPAD_PORT_1);
      joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

      //Music
      music_play();

     // Level Switch Logic
     if (level_update(&player, zombies, zombie_count, &enemy_count, &level)) {
      level_start_time = get_time_s(); // Record new level start time
      // Update Zombie Counts at Level change
      current_level_index = (current_level_index + 1) % TOTAL_LEVELS; // Increment the level index and reset to 0
      level = levels[current_level_index]; // Update which level on
      zombie_count = level->zombie_count; // Update Zombie count based on level
      continue; // Skip drawing this frame
    };
  
      // Update Player
      player_update(&player, deltaTime, joypad, btn, zombies, zombie_count);

      // Update Particle
      //Hard Code Map Boundary for now also used by player (140.f)
      //bullet_update(&bullet, &player.position, player.rotation_y, 140.0f, zombies, &zombie_count, &enemy_count, btn);
      bullet_update(&bullet, &player.position, player.rotation_y, zombies, &zombie_count, &enemy_count);
      
      // Update Zombie
      zombie_update_all(zombies, zombie_count, &player.position, deltaTime);

      // Update Camera
      camera_update(&camera, &player.position, player.rotation_y);
      if (btn.l) camera_toggle_mode(&camera); // Swtich between views
      // Update Camera

      if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame
    
      // ======== Draw (3D) ======== //
      rdpq_attach(display_get(), display_get_zbuf());

      t3d_frame_start();
      t3d_viewport_attach(camera_get_viewport(&camera));

      t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
      t3d_screen_clear_depth();

      t3d_light_set_ambient(colorAmbient);
      t3d_light_set_directional(0, colorDir, &lightDirVec);
      t3d_light_set_count(1);

      syncPoint = rspq_syncpoint_new();

      T3DViewport *vp = camera_get_viewport(&camera);

      // Draw order Matters!!!!!!
      // Level End logic
      if (enemy_count == 0) {
          // Somehow figure out a fade in for the arrows?????
          // Draw (Map With Portal)
          map_draw(&map);
          map_draw(&mapPortal);

          /* SYNC PIPE ISSUE

          // Calculate Arrow Postition 
          T3DVec3 base_world_pos = map.position;
          base_world_pos.v[0] += 3.0f; // Adjust X if needed

          const float width = 15.0f;
          const float scale = 0.3f;
          const float spacing = 15.0f;
          // New
          const float speed   = 0.4f;  // adjust for faster/slower scrolling
          const float base_y  = -110.0f;

          // Accumulator for arrow movement (declared static to persist between frames)
          static float arrow_scroll_offset = 0.0f;
          arrow_scroll_offset += speed;
          if (arrow_scroll_offset >= spacing * 3.0f) {
            arrow_scroll_offset -= spacing * 3.0f;
          };
          // End New
          
          // Scale the Arrow down
          rdpq_blitparms_t parms = {
              .scale_x = scale,
              .scale_y = scale,
          };

          // Draw (Arrow (X3) loop)
          for (int i = 0; i < 3; i++) {
              T3DVec3 arrow_world = base_world_pos;
              //arrow_world.v[2] -= 120.0f + i * spacing;
              float offset = fmodf(arrow_scroll_offset + i * spacing, spacing * 3.0f);
              arrow_world.v[2] += base_y - offset;

              T3DVec3 screen_pos;
              t3d_viewport_calc_viewspace_pos(vp, &screen_pos, &arrow_world);

              float x0 = screen_pos.v[0] - width / 2.0f;
              float y0 = screen_pos.v[1];

              // Draw (Arrow)
              // This is needed for sprites with a transparent background
              rdpq_set_mode_standard();
              rdpq_mode_alphacompare(1);
              rdpq_sprite_blit(arrow, x0, y0, &parms);
              // To make textures on meshes scale properly (Due to rdpq_set_mode_standard();)
              rdpq_mode_persp(true);
              
          }; */

      } else {
          // Draw (Regular Map)
          map_draw(&mapWall);
          map_draw(&map);
      };

      // Draw (Banners)
      for (int i = 0; i < zombie_count; i++) {
        
        // 1. Draw BLOOD banner if zombie is dead
        if (!zombies[i].alive) {

        float time_since_death = get_time_s() - zombies[i].blood_time;

        // Compute scale: start from (3.0, 0.3), shrink by 0.1 per second
        zombies[i].blood_scale -= 0.01f * time_since_death;

        // Clamp: If scale reaches zero or below, skip drawing
        if (zombies[i].blood_scale <= 0.0f) {
            continue;  // Skip this blood banner
        };        

          t3d_mat4fp_from_srt_euler(&blood_matrices[i],
              (float[3]){zombies[i].blood_scale, 0.3f, zombies[i].blood_scale},  // scale: X, Z, Y
              (float[3]){0, 0, 0},
              (float[3]){zombies[i].position.v[0], 0, zombies[i].position.v[2]});

          //rdpq_mode_persp(true);
          draw_floor_banner(&blood_matrices[i], BANNER_BLOOD);
      };
      
        // 2. Draw SPAWN banner (only during first 4 seconds)
        if (get_time_s() - level_start_time < 4.0f) {

          t3d_mat4fp_from_srt_euler(&spawn_matrices[i],
            (float[3]){0.3f, 0.3f, 0.3f},
            (float[3]){0, 0, 0},
            (float[3]){level->zombies[i].position.v[0], 0, level->zombies[i].position.v[2]});
            
            draw_floor_banner(&spawn_matrices[i], BANNER_SPAWN);
          
        };

      };

      // Draw (Bullets)
      bullet_draw(&bullet);
      
      // Draw (Player)
      player_draw(&player); // Issue with player and zomebie draw. Maybe due to camera??????

      // Draw (Zombies)
      zombie_draw_all(zombies, zombie_count);

      /* 
      SYNC PIPE ISSUE
      // Draw Health Bar (Zombie)
      for (int i = 0; i < zombie_count; i++) {
        draw_zombie_health_bar(&zombies[i], vp);      // 2D bar
      }
      */

      /*
      SYNY PIPE ISSUE
      // Draw Health Bar (Slayer)
      draw_player_health_bar(&player, vp);
      */

      // ======== Draw (UI) ======== //
      float posX = 16;
      float posY = 24;

      rdpq_sync_pipe();

      posY = 216;
      // MEMORY TRACKING
      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 15, "Mem: %d KiB", heap_stats.used/1024); // get memory usage
      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "FPS: %.2f", display_get_fps()); posY += 10; // Get FPS
  
      // LEVEL
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Map (X, Y): (%.4f, %.4f)",  map.position.v[0], map.position.v[2]); posY += 10; //Displays position
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Positions (X, Y): (%d, %d)", joypad.stick_x, joypad.stick_y); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Enemy Count: (%d)", enemy_count); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Total Levels: (%d)", TOTAL_LEVELS); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Level Index: (%d)", current_level_index); posY += 10;

      // BULLET
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Bullet Position (X, Y): (%.4f, %.4f)", bullet.position.v[0], bullet.position.v[2]); posY += 10; //Displays position
      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Bullet Rotation: %.4f", bullet.rotation_y); posY += 10;

      //TIME
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "After Death time: (%.4f)", get_time_s() - zombies[0].blood_time); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Game Time: (%.4f)", get_time_s()); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Reset Time: (%.4f)", get_time_s() - level_start_time); posY += 10;
      

      //SLAYER
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Player Positions (X, Y): (%.4f, %.4f)", player.position.v[0], player.position.v[2]); posY += 10; //Displays position
      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Slayer Rotation (Y):%.4f", player.rotation_y); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Speed: %.4f", currSpeed); posY += 10; //Speed

      // ZOMBIE
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie blood scale: (%.4f)",zombies[1].blood_scale - 0.01f * (get_time_s() - zombies[0].blood_time)); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie Count: (%d)", zombie_count); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie Death time:%.4f", zombies[0].blood_time); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie Life: (%d, %d)", zombies[0].health, zombies[1].health); posY += 10;
      //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie Alive: (%d, %d)", zombies[0].alive, zombies[1].alive); posY += 10;


      rdpq_detach_show();

      // for now, press start to go back to menu
      if (btn.start) {
        state = STATE_MENU;
      };

    }

    // Free Assests to prevent memory leaks
    // Music Stop
    music_stop();
    
    // Bullets Cleanup
    bullet_cleanup(&bullet);
    
    // Map Cleanup
    map_destroy(&map);
    map_destroy(&mapWall);
    map_destroy(&mapPortal);
    
    // Player Cleanup
    player_cleanup(&player);
    
    // Sprite Cleanup
    //sprite_free(arrow);

    // Zombie(s) Cleanup
    zombie_destroy_all(zombies, zombie_count);

    // Banner(s) Cleanup
    banners_destroy();
    free(spawn_matrices);
    free(blood_matrices);

}

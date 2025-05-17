// General file to hold all libraries
#include "libs.h"
// files per library
#include "slayer.h"
#include "bullets.h"
#include "map.h"
#include "camera.h"
#include "zombie.h"
#include "levels.h"
#include "health.h"
//#include <mikmod.h> // Music
#include <audio.h>

// Needed for syncing?????
float get_time_s() {
  return (float)((double)get_ticks_us() / 1000000.0);
}

int main()
{
  debug_init_isviewer();
  debug_init_usblog();
  asset_init_compression(2);

  dfs_init(DFS_DEFAULT_LOCATION);

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);

  rdpq_init();
  joypad_init();

  t3d_init((T3DInitParams){});

  //Music
	audio_init(44100, 20);
	mixer_init(32);
	mixer_ch_set_limits(6, 0, 128000, 0);
	dfs_init(DFS_DEFAULT_LOCATION);

  xm64player_t xm;
	xm64player_open(&xm, "rom:/mallard_game_music.xm64");
	xm64player_play(&xm, 0);
/*
	while(1) {
		if (audio_can_write()) {
			short *buf = audio_write_begin();
			mixer_poll(buf, audio_get_buffer_length());
			audio_write_end();
		}
	}
};*/

//  if (music) mod_play(music, true);
  //Music
  
  //particles
  // Init tpx here, same as t3d, you can pass in a custom matrix-stack size if needed.
  tpx_init((TPXInitParams){});
  // Allocate a particle buffer.
  // In contrast to triangles, there is no split between loading and drawing.
  // So later there will be only one command to draw all of them in one go.
  // Meaning you only have to allocate an buffer of arbitrary size here and fill it with data.

  // Font
  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));


  T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
  t3d_vec3_norm(&lightDirVec);
  uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
  uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

  T3DMat4FP *modelMatFP = malloc_uncached(sizeof(T3DMat4FP));

   // Level definition
  const LevelData *level = &LEVEL_1;

  // Initialize Camera
  Camera camera;
  camera_init(&camera);
  // Initialize Camera

  // Initialize Particles
  BulletSystem bullet;
  bullet_init(&bullet, 10, 0.4f, 0.9f);
  // Initialize Particles

  // Initialize Map
  Map map;
  map_init(&map);
  rspq_block_begin();
  map_draw(&map);
  rspq_block_t *dplMap = rspq_block_end();
  // Initialize Map

  // Initialize Player
  Player player;
  player_init(&player, modelMatFP);
  player.position = level->player.position;
  player.rotation_y = level->player.rotation_y;
  rspq_block_begin();
  player_draw(&player);
  rspq_block_t *dplSlayer = rspq_block_end();
  // Initialize Player

  // Initialize Zombie
  int zombie_count = level->zombie_count;
  Zombie *zombies = malloc_uncached(sizeof(Zombie) * zombie_count);
  for (int i = 0; i < zombie_count; i++) {
      zombie_init(&zombies[i], &level->zombies[i].position);
      zombies[i].rotation_y = level->zombies[i].rotation_y;
  }
  // Initialize Zombie

  float lastTime = get_time_s() - (1.0f / 60.0f);
  rspq_syncpoint_t syncPoint = 0;

  for(;;)
  {
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
    	// Check if there are any free audio buffers
	if (audio_can_write()) {
		// Select an audio buffer that we can write to
		short *buf = audio_write_begin();
		// Write to the audio buffer from the mixer
		mixer_poll(buf, audio_get_buffer_length());
		// Tell the audio system that the buffer has
		// been filled and is ready for playback
		audio_write_end();
	};
  // Music

    enemy_prune_dead(zombies, &zombie_count);

    // Update Player
    player_update(&player, deltaTime, joypad, btn, zombies, zombie_count);
    // Update Particle
    bullet_update(&bullet, &player.position, player.rotation_y, 140.0f, zombies, &zombie_count, btn);
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

    rspq_block_run(dplMap);
    rspq_block_run(dplSlayer);
    //rspq_block_run(dplZombie);

    syncPoint = rspq_syncpoint_new();

    // Draw (Particles)
    bullet_draw(&bullet);
     // Draw (Zombies)
    zombie_draw_all(zombies, zombie_count);

    // ======== Draw (UI) ======== //
    float posX = 16;
    float posY = 24;

    rdpq_sync_pipe();
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "[A] Attack: %d", isAttack);

    posY = 216;
    // Display Options
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Speed: %.4f", currSpeed); posY += 10; //Speed
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 15, "Mem: %d KiB", heap_stats.used/1024); // get memory usage
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Player Positions (X, Y): (%.4f, %.4f)", player.position.v[0], player.position.v[2]); posY += 10; //Displays position
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Particle Positions (X, Y): (%.4f, %.4f)", bullet.bullets[0].position.v[0], bullet.bullets[0].position.v[2]); posY += 10; //Displays position
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Slayer Rotation (Y):%.4f", player.rotation_y); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Particle Rotation: %.4f", particles.rotation.v[1]); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Positions (X, Y): (%d, %d)", joypad.stick_x, joypad.stick_y); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie Count: (%d)", zombie_count); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie Life: (%d, %d)", zombies[0].health, zombies[1].health); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Zombie Alive: (%d, %d)", zombies[0].alive, zombies[1].alive); posY += 10;


    // Get FPS
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "FPS: %.2f", display_get_fps()); posY += 10;

    rdpq_detach_show();
  }

  player_cleanup(&player);
  bullet_cleanup(&bullet);
  map_destroy(&map);
  //zombie_destroy(&zombie);
  zombie_destroy_all(zombies, zombie_count);
  t3d_destroy();
  
  // music
  //mod_stop();
  //mod_free(music);
  //mod_close();
  // music
  
  
  return 0;
}


#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#include <t3d/tpx.h> // Particles

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
  
  //particles
  // Init tpx here, same as t3d, you can pass in a custom matrix-stack size if needed.
  tpx_init((TPXInitParams){});
  // Allocate a particle buffer.
  // In contrast to triangles, there is no split between loading and drawing.
  // So later there will be only one command to draw all of them in one go.
  // Meaning you only have to allocate an buffer of arbitrary size here and fill it with data.
  uint32_t particleCountMax = 100'000;
  uint32_t particleCount = 2; // Cannot be odd
  // NOTE: just like with vertices, particles are interleaved in pairs of 2.
  // So one TPXParticle struct always contains 2 particles.
  // If you need an odd number, just set the second particle size to 0.
  uint32_t allocSize = sizeof(TPXParticle) * particleCountMax / 2;
  TPXParticle *particles = malloc_uncached(allocSize);
  debugf("Particle-Buffer %ldkb\n", allocSize / 1024);
  
  //T3DMat4FP *matFP = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP *matPartFP = malloc_uncached(sizeof(T3DMat4FP));

  float partSizeX = 0.4f;
  float partSizeY = 0.9f;
  
  int8_t *ptPos = particles[0].posA;
  uint8_t *ptColor = particles[0].colorA;
  particles[0].sizeA = 50;
  ptPos[0] = ptPos[0] + 1; // Y position
  ptPos[1] = 20;// // Z position
  ptPos[2] = 0; // X postion
  ptColor[3] = 8; // alpha (transparency)
  ptColor[0] = 0; // Red
  ptColor[1] = 0; // Green
  ptColor[2] = 255; // Blue
  //particles

  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));
  T3DViewport viewport = t3d_viewport_create();

  // Map
  T3DMat4FP* modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0, 0, -10});

  // Camera Postition
  //                 X     Y       Z
  T3DVec3 camPos = {{0.0f, 125.0f, 135.0f}}; //-90.0f
  // Set to origin
  T3DVec3 camTarget = {{0.0f, 0.0f, 0.0f}};

  T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
  t3d_vec3_norm(&lightDirVec);

  uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
  uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

  // Map
  T3DModel *modelMap = t3d_model_load("rom:/map.t3dm");
  
  // Player
  T3DModel *modelShadow = t3d_model_load("rom:/shadow.t3dm");
  T3DModel *model = t3d_model_load("rom:/slayer.t3dm");
  // Player

  // First instantiate skeletons, they will be used to draw models in a specific pose
  // And serve as the target for animations to modify
  T3DSkeleton skel = t3d_skeleton_create(model);
  T3DSkeleton skelBlend = t3d_skeleton_clone(&skel, false); // optimized for blending, has no matrices

  // Now create animation instances (by name), the data in 'model' is fixed,
  // whereas 'anim' contains all the runtime data.
  // Note that tiny3d internally keeps no track of animations, it's up to the user to manage and play them.
  //T3DAnim animIdle = t3d_anim_create(model, "breathing-idle");
  //t3d_anim_attach(&animIdle, &skel); // tells the animation which skeleton to modify

  T3DAnim animIdle = t3d_anim_create(model, "breathing-idle");
  t3d_anim_attach(&animIdle, &skel); // tells the animation which skeleton to modify

  T3DAnim animWalk = t3d_anim_create(model, "walking-left");
  t3d_anim_attach(&animWalk, &skelBlend);
  // multiple animations can attach to the same skeleton, this will NOT perform any blending
  // rather the last animation that updates "wins", this can be useful if multiple animations touch different bones
  
  //T3DAnim animAttack = t3d_anim_create(model, "Snake_Attack");
  //t3d_anim_set_looping(&animAttack, false); // don't loop this animation
  //t3d_anim_set_playing(&animAttack, false); // start in a paused state
  //3d_anim_attach(&animAttack, &skel);

  rspq_block_begin();
    t3d_matrix_push(modelMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw_skinned(model, &skel); // as in the last example, draw skinned with the main skeleton

    rdpq_set_prim_color(RGBA32(0, 0, 0, 120));
    t3d_model_draw(modelShadow);
    t3d_matrix_pop(1);
  rspq_block_t *dplSnake = rspq_block_end();

  // Map
  rspq_block_begin();
    t3d_matrix_push(mapMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(modelMap);
    t3d_matrix_pop(1);
  rspq_block_t *dplMap = rspq_block_end();


  float lastTime = get_time_s() - (1.0f / 60.0f);
  rspq_syncpoint_t syncPoint = 0;

  T3DVec3 moveDir = {{0,0,0}}; // Player
  T3DVec3 playerPos = {{0,0.15f,0}}; // Player

  float rotY = 0.0f;
  float currSpeed = 0.0f;
  float animBlend = 0.0f;
  //bool isAttack = false;

  // This is used to switch between the camea views. if false (top down) ir true (behind player)
  bool pov = false;

  //particles
  float partMatScaleVal = 0.8f;
  T3DVec3 particleMatScale = {{1, 1, 1}};
  T3DVec3 particlePos = {{0, 0, 0}};
  T3DVec3 particleRot = {{0, 0, 0}};
  float xSpeed =0.0f;
  float ySpeed = 1.0f;
  //particles

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

    T3DVec3 newDir = {{
       (float)joypad.stick_x * 0.05f, 0,
      -(float)joypad.stick_y * 0.05f
    }};
    float speed = sqrtf(t3d_vec3_len2(&newDir));

    // Player Attack
    //if((btn.a || btn.b) && !animAttack.isPlaying) {
    //  t3d_anim_set_playing(&animAttack, true);
    //  t3d_anim_set_time(&animAttack, 0.0f);
    //  isAttack = true;
   // }

    // Player movement
    if(speed > 0.15f) {
      newDir.v[0] /= speed;
      newDir.v[2] /= speed;
      moveDir = newDir;

      float newAngle = atan2f(moveDir.v[0], moveDir.v[2]);
      rotY = t3d_lerp_angle(rotY, newAngle, 0.25f);
      currSpeed = t3d_lerp(currSpeed, speed * 0.15f, 0.15f);
    } else {
      currSpeed *= 0.8f;
    }

    // use blend based on speed for smooth transitions
    animBlend = currSpeed / 0.51f;
    if(animBlend > 1.0f)animBlend = 1.0f;

    // move player...
    playerPos.v[0] += moveDir.v[0] * currSpeed;
    playerPos.v[2] += moveDir.v[2] * currSpeed;
    
    // ...and limit position inside the box
    const float BOX_SIZE = 140.0f;
    if(playerPos.v[0] < -BOX_SIZE)playerPos.v[0] = -BOX_SIZE;
    if(playerPos.v[0] >  BOX_SIZE)playerPos.v[0] =  BOX_SIZE;
    if(playerPos.v[2] < -BOX_SIZE)playerPos.v[2] = -BOX_SIZE;
    if(playerPos.v[2] >  BOX_SIZE)playerPos.v[2] =  BOX_SIZE;

    if(btn.b)pov = !pov;
    // Default Overhead view
    if(pov == false) {
      // position the camera above the player
      camTarget = playerPos;
      camPos.v[0] = camTarget.v[0]; // X position
      camPos.v[1] = camTarget.v[1] + 180; // Move the camera up and down
      camPos.v[2] = camTarget.v[2]; // Y position

      t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 10.0f, 150.0f);
      
      // Setup View Matrix
      t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,0,-1}});

    }

    // Set View behind play if B is Pressed
    else {
      // position the camera behind the player
      camTarget = playerPos;
      camTarget.v[2] -= 20;
      camPos.v[0] = camTarget.v[0];
      camPos.v[1] = camTarget.v[1] + 45;
      camPos.v[2] = camTarget.v[2] + 65;

      t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 10.0f, 150.0f);
      t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
    }

    // Update the animation and modify the skeleton, this will however NOT recalculate the matrices
    t3d_anim_update(&animIdle, deltaTime);
    t3d_anim_set_speed(&animWalk, animBlend + 0.15f);
    t3d_anim_update(&animWalk, deltaTime);

    //if(isAttack) {
    //  t3d_anim_update(&animAttack, deltaTime); // attack animation now overrides the idle one
    //  if(!animAttack.isPlaying)isAttack = false;
    //}

    // We now blend the walk animation with the idle/attack one
    t3d_skeleton_blend(&skel, &skel, &skelBlend, animBlend);

    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

    // Now recalc. the matrices, this will cause any model referencing them to use the new pose
    t3d_skeleton_update(&skel);

    // Update player matrix
    // Update player matrix
    t3d_mat4fp_from_srt_euler(modelMatFP,
    (float[3]){0.0035f, 0.0035f, 0.0035f},  // Scale
    // Rotation
    (float[3]){-4.5f, // Rotate around the X axis
                   0, // Rotate around Z axis
              -rotY}, // Rotate around Y axis
    //Position
    (float[3]){playerPos.v[0], // X (Positive: Left, Max: 265 Negative: Right Min: -265. Measured from middle of object? Based on Blender Origin
               playerPos.v[1], // Z (Positive: Up, Negative: Down))
               playerPos.v[2] // Y (Positive: Up, Max: 360 / Negative: Down, Min: -360 Image Warped. Measured from middle of object? Based on Blender Origin
        }  
    );


    //particles
    particleMatScale = (T3DVec3){{partMatScaleVal, partMatScaleVal, partMatScaleVal}};
    //Update particle Movement
    //generate_particles_random(particles, particleCount);


    rdpq_set_env_color((color_t){0xFF, 0xFF, 0xFF, 0xFF});

    // Check if particle will hit a wall, if so reset to current player postion and resume movement
    if(particlePos.v[0] < -BOX_SIZE || particlePos.v[0] >  BOX_SIZE) {
          particlePos.v[0]  = playerPos.v[0]; // Update X Location
          particlePos.v[2]  = playerPos.v[2]; // Update Y Location
    }
    if(particlePos.v[2] < -BOX_SIZE || particlePos.v[2] >  BOX_SIZE) {
      particlePos.v[0]  = playerPos.v[0]; // Update X Location
      particlePos.v[2]  = playerPos.v[2]; // Y Direction
    };
    
    // If the bullet has been reset to player position reset bullet speed
    if(particlePos.v[0] == playerPos.v[0] && particlePos.v[2] == playerPos.v[2]) {
    xSpeed = sin(rotY); // X direction
    ySpeed = cos(rotY); // Y direction
    }

    // Update the particle position (Speed is constant)
    particlePos.v[0] = particlePos.v[0] + xSpeed; // X direction
    particlePos.v[2] = particlePos.v[2] + ySpeed; // Y direction

     t3d_mat4fp_from_srt_euler(matPartFP, 
      particleMatScale.v, 
      // Rotation
      particleRot.v,
          //Position
    (float[3]){particlePos.v[0], // X (Positive: Left, Max: 265 Negative: Right Min: -265. Measured from middle of object? Based on Blender Origin
               particlePos.v[1], // Z (Positive: Up, Negative: Down))
               particlePos.v[2] // Y (Positive: Up, Max: 360 / Negative: Down, Min: -360 Image Warped. Measured from middle of object? Based on Blender Origin
} 
    );
//particles

    // ======== Draw (3D) ======== //
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    rspq_block_run(dplMap);
    rspq_block_run(dplSnake);

    syncPoint = rspq_syncpoint_new();


    // ======== Draw (Particles) ======== //

    // Prepare drawing particles.
    // In contrast to t3d which draws triangles, tpx will emit screen-space rectangles.
    // The color of each particle is set as prim. color, and shade is not defined.
    // So we have to set up a few things via rdpq to make that work depending on the desired effect.
    //
    // In our case, we want to combine it with env. color in the CC.
    // In order to have depth, you also need to enable `rdpq_mode_zoverride` so the ucode can set this correctly.
    rdpq_sync_pipe();
    rdpq_sync_tile();
    rdpq_set_mode_standard();
    rdpq_mode_zbuf(true, true);
    rdpq_mode_zoverride(true, 0, 0);
    rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,0,ENV,0), (0,0,0,1)));

    // tpx is its own ucode, so nothing that was set up in t3d carries over automatically.
    // For convenience, you can call 'tpx_state_from_t3d' which will copy over
    // the current matrix, screen-size and w-normalization factor.
    tpx_state_from_t3d();
    // tpx also has the same matrix stack functions as t3d, Note that the stack itself is NOT shared
    // so any push/pop here will not affect t3d and vice versa.
    // Also make sure that the first stack operation you do after 'tpx_state_from_t3d' is a push and not a set.
    tpx_matrix_push(matPartFP);
    // While each particle has its own size, there is a global scaling factor that can be set.
    // This can only scale particles down, so the range is 0.0 - 1.0.
    tpx_state_set_scale(partSizeX, partSizeY);

    // Now draw particles. internally this will load, transform and draw them in one go on the RSP.
    // While the ucode can only handle a 344 at a time, this function will automatically batch them
    // so you can specify an arbitrary amount of particles (as long as it's an even count)
    tpx_particle_draw(particles, particleCount);

    // Make sure end up at the same stack level as before.
    tpx_matrix_pop(1);

    // After all particles are drawn, there is nothing special to do.
    // You can either continue with t3d (remember to revert rdpq settings again) or do other 2D draws.


    // ======== Draw (UI) ======== //
    float posX = 16;
    float posY = 24;

    rdpq_sync_pipe();
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "[A] Attack: %d", isAttack);

    posY = 216;
    // Display Options
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Speed: %.4f", currSpeed); posY += 10; //Speed
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 15, "Mem: %d KiB", heap_stats.used/1024); // get memory usage
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Player Positions (X, Y): (%.4f, %.4f)", playerPos.v[0], playerPos.v[2]); posY += 10; //Displays position
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Particle Positions (X, Y): (%.4f, %.4f)", particlePos.v[0], particlePos.v[2]); posY += 10; //Displays position
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Position (Y):%.4f", playerPos.v[2]); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Position (Z): %.4f", playerPos.v[1]); posY += 10;
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "Positions (X, Y): (%d, %d)", joypad.stick_x, joypad.stick_y); posY += 10;

    // Get FPS
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "FPS: %.2f", display_get_fps()); posY += 10;

    rdpq_detach_show();
  }

  t3d_skeleton_destroy(&skel);
  t3d_skeleton_destroy(&skelBlend);

  //t3d_anim_destroy(&animIdle);
  t3d_anim_destroy(&animWalk);
  //t3d_anim_destroy(&animAttack);

  t3d_model_free(model);
  t3d_model_free(modelMap);
  t3d_model_free(modelShadow);

  t3d_destroy();
  return 0;
}


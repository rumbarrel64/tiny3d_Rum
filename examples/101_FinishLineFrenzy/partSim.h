#pragma once

static void generate_particles_random(TPXParticle *particles, uint32_t count) {
  for (int i = 0; i < count; i++) {

    int8_t *ptPos = particles[i].posA;
    uint8_t *ptColor = particles[i].colorA;

    particles[i].sizeA = 50; // + (rand() % 10);
    particles[i].sizeB = 0; // + (rand() % 10);

    ptPos[0] = ptPos[i]  + 5; // Y position
    ptPos[1] = 20;// // Z position
    ptPos[2] = ptPos[i] + 5; // X postion

    ptColor[3] = 8;//rand() % 8; // alpha (transparency)
    ptColor[0] = 255; // + (rand() % 230);
    ptColor[1] = 255; // + (rand() % 230);
    ptColor[2] = 255; // + (rand() % 230);
  }
}

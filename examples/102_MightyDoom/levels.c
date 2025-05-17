// levels.c
#include "levels.h"

const LevelData LEVEL_1 = {
    .player = {
        // Initial Position and direction (rotation) of slayer
        .position = {{0.0f, 0.15f, 104.0f}}, // X, Z, Y
        .rotation_y = 3.1416f
    },
    .zombie_count = 2, // Increase the zombie count here
    .zombies = {
        { .position = {{  0.0f, 0.15f, -96.0f }}, .rotation_y = 0.0f },
        { .position = {{ 82.0f, 0.15f, -123.0f }}, .rotation_y = 0.0f },
        { .position = {{-87.0f, 0.15f, -123.0f }}, .rotation_y = 0.0f }
    }
};

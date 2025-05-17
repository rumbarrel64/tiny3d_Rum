// zombie_status.c
#include "health.h"

void enemy_prune_dead(Zombie *zombies, int *zombie_count) {
    for (int i = 0; i < *zombie_count; i++) {
        if (zombies[i].health <= 0) {
            // Mark as not alive just for consistency (optional)
            zombies[i].alive = false;

            // Shift remaining zombies left to compact array
            for (int j = i; j < *zombie_count - 1; j++) {
                zombies[j] = zombies[j + 1];
            }
            (*zombie_count)--;
            i--; // Stay on the new zombie at this index
        }
    }
}
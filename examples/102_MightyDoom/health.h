// health.h
#pragma once

#include "zombie.h"

// Updates the zombie array and count in-place based on health
void enemy_prune_dead(Zombie *zombies, int *zombie_count);
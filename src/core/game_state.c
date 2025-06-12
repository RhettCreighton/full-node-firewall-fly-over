/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/core/game_state.h"
#include "sky_combat/models/aircraft.h"
#include <stdlib.h>
#include <raylib.h>

struct game_state_s {
    bool running;
    aircraft_t* player_aircraft;
    float game_time;
};

game_state_t* game_state_create(void) {
    game_state_t* state = calloc(1, sizeof(game_state_t));
    if (!state) return NULL;
    
    state->running = true;
    state->player_aircraft = aircraft_create((Vector3){0, 100, 0});
    state->game_time = 0;
    
    return state;
}

void game_state_destroy(game_state_t* state) {
    if (!state) return;
    if (state->player_aircraft) aircraft_destroy(state->player_aircraft);
    free(state);
}

void game_state_update(game_state_t* state, float dt) {
    if (!state) return;
    state->game_time += dt;
}

bool game_state_is_running(game_state_t* state) {
    return state && state->running;
}
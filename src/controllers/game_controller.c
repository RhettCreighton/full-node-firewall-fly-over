/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/controllers/game_controller.h"
#include <stdlib.h>
#include <raylib.h>

struct game_controller_s {
    game_state_t* state;
};

game_controller_t* game_controller_create(game_state_t* state) {
    game_controller_t* controller = calloc(1, sizeof(game_controller_t));
    if (!controller) return NULL;
    
    controller->state = state;
    return controller;
}

void game_controller_destroy(game_controller_t* controller) {
    free(controller);
}

void game_controller_update(game_controller_t* controller, float dt) {
    if (!controller) return;
    
    // Handle keyboard input for now
    float move_x = 0, move_y = 0;
    
    if (IsKeyDown(KEY_A)) move_x = -1;
    if (IsKeyDown(KEY_D)) move_x = 1;
    if (IsKeyDown(KEY_W)) move_y = -1;
    if (IsKeyDown(KEY_S)) move_y = 1;
    
    // Update game based on input
}
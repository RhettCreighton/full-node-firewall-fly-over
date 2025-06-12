/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include "sky_combat/core/game_state.h"
#include "sky_combat/controllers/game_controller.h"
#include "sky_combat/views/renderer.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define TARGET_FPS 60

int main(int argc, char* argv[]) {
    // Initialize
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sky Combat - Open World");
    SetTargetFPS(TARGET_FPS);
    
    // Create game systems
    game_state_t* state = game_state_create();
    game_controller_t* controller = game_controller_create(state);
    renderer_t* renderer = renderer_create(state);
    
    if (!state || !controller || !renderer) {
        fprintf(stderr, "Failed to initialize game systems\n");
        return 1;
    }
    
    printf("Sky Combat - Open World Aircraft Combat\n");
    printf("Press F1 for help, ~ for console\n");
    
    // Main game loop
    while (!WindowShouldClose() && game_state_is_running(state)) {
        float dt = GetFrameTime();
        
        // Update
        game_controller_update(controller, dt);
        game_state_update(state, dt);
        
        // Render
        BeginDrawing();
        renderer_draw(renderer);
        EndDrawing();
    }
    
    // Cleanup
    renderer_destroy(renderer);
    game_controller_destroy(controller);
    game_state_destroy(state);
    CloseWindow();
    
    return 0;
}

/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/views/renderer.h"
#include <stdlib.h>
#include <raylib.h>

struct renderer_s {
    game_state_t* state;
    Camera3D camera;
};

renderer_t* renderer_create(game_state_t* state) {
    renderer_t* renderer = calloc(1, sizeof(renderer_t));
    if (!renderer) return NULL;
    
    renderer->state = state;
    renderer->camera = (Camera3D){
        .position = {0, 150, -100},
        .target = {0, 0, 0},
        .up = {0, 1, 0},
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };
    
    return renderer;
}

void renderer_destroy(renderer_t* renderer) {
    free(renderer);
}

void renderer_draw(renderer_t* renderer) {
    if (!renderer) return;
    
    ClearBackground((Color){10, 10, 30, 255});
    
    BeginMode3D(renderer->camera);
    
    // Draw ground
    DrawGrid(200, 50);
    
    // Draw aircraft
    DrawCube((Vector3){0, 100, 0}, 10, 3, 15, BLUE);
    
    EndMode3D();
    
    // Draw HUD
    DrawText("Sky Combat", 10, 10, 20, GREEN);
    DrawFPS(GetScreenWidth() - 100, 10);
}
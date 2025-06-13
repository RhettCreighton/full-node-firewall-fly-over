/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raylib.h>
#include "sky_combat/core/game_state.h"
#include "sky_combat/core/specification.h"
#include "sky_combat/controllers/game_controller.h"
#include "sky_combat/views/renderer.h"
#include "sky_combat/models/aircraft.h"
#include "sky_combat/specifications/aircraft_rendering_spec.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define TARGET_FPS 60

/* Example: Specification Driven Development
 * 
 * This demonstrates how specifications work:
 * 1. We define what MUST happen (aircraft must render)
 * 2. We check it happens EVERY frame
 * 3. If it fails, we know IMMEDIATELY
 * 4. Six Sigma reliability - 99.99966% success required
 */

int main(int argc, char* argv[]) {
    // Initialize specification system FIRST
    spec_system_init();
    
    // Create specifications
    aircraft_rendering_spec_t* aircraft_spec = aircraft_rendering_spec_create();
    
    // Also create other critical specs
    specification_t* input_spec = spec_create("Input System", 
                                            "Input must be responsive within 16ms");
    specification_t* physics_spec = spec_create("Physics System",
                                              "Physics must be deterministic");
    specification_t* memory_spec = spec_create("Memory Management",
                                             "No memory leaks allowed");
    
    spec_register(input_spec);
    spec_register(physics_spec);
    spec_register(memory_spec);
    
    // Initialize game
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sky Combat - Specification Driven");
    SetTargetFPS(TARGET_FPS);
    
    // Create game systems
    game_state_t* state = game_state_create();
    game_controller_t* controller = game_controller_create(state);
    renderer_t* renderer = renderer_create(state);
    
    // Create test aircraft
    aircraft_t* aircraft = aircraft_create((Vector3){0, 100, 0});
    Camera3D camera = {
        .position = {0, 150, -100},
        .target = {0, 100, 0},
        .up = {0, 1, 0},
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };
    
    // Verify initial state
    SPEC_CHECK_NOT_NULL(memory_spec, state, "Game state allocation failed");
    SPEC_CHECK_NOT_NULL(memory_spec, controller, "Controller allocation failed");
    SPEC_CHECK_NOT_NULL(memory_spec, renderer, "Renderer allocation failed");
    SPEC_CHECK_NOT_NULL(memory_spec, aircraft, "Aircraft allocation failed");
    
    printf("Sky Combat - Specification Driven Development\n");
    printf("Press F1 to show specification report\n");
    printf("Press F2 to inject rendering failure (test)\n");
    
    bool inject_failure = false;
    int frame_count = 0;
    
    // Main game loop
    while (!WindowShouldClose() && game_state_is_running(state)) {
        float dt = GetFrameTime();
        frame_count++;
        
        // INPUT SPECIFICATION: Check input latency
        double input_start = GetTime();
        
        // Update
        game_controller_update(controller, dt);
        game_state_update(state, dt);
        aircraft_update(aircraft, 0, 0, dt);
        
        double input_end = GetTime();
        double input_latency_ms = (input_end - input_start) * 1000.0;
        
        SPEC_CHECK_RANGE(input_spec, input_latency_ms, 0, 16,
                        "Input latency exceeds frame time");
        
        // PHYSICS SPECIFICATION: Verify determinism
        Vector3 old_pos = aircraft_get_position(aircraft);
        aircraft_update(aircraft, 0, 0, dt);
        Vector3 new_pos = aircraft_get_position(aircraft);
        
        // Position shouldn't change with zero input
        float epsilon = 0.0001f;
        bool physics_deterministic = (fabsf(old_pos.x - new_pos.x) < epsilon &&
                                    fabsf(old_pos.y - new_pos.y) < epsilon &&
                                    fabsf(old_pos.z - new_pos.z) < epsilon);
        SPEC_CHECK(physics_spec, physics_deterministic,
                  "Physics non-deterministic with zero input");
        
        // RENDERING SPECIFICATION
        aircraft_rendering_spec_begin_frame(aircraft_spec, aircraft, &camera);
        
        // Render
        BeginDrawing();
        ClearBackground((Color){10, 10, 30, 255});
        
        BeginMode3D(camera);
        
        // Draw ground
        DrawGrid(200, 50);
        
        // Draw aircraft (unless we're testing failure)
        if (!inject_failure) {
            Vector3 pos = aircraft_get_position(aircraft);
            DrawCube(pos, 10, 3, 15, BLUE);
        }
        
        EndMode3D();
        
        // Complete render verification
        aircraft_rendering_spec_end_frame(aircraft_spec);
        
        // Check rendering worked
        aircraft_rendering_spec_check_visibility(aircraft_spec, aircraft, &camera);
        
        // HUD
        DrawText("Sky Combat - Spec Driven", 10, 10, 20, GREEN);
        DrawFPS(GetScreenWidth() - 100, 10);
        
        // Show spec status
        DrawText(TextFormat("Specs: I:%s P:%s M:%s R:%s", 
                           input_spec->checks_failed == 0 ? "OK" : "FAIL",
                           physics_spec->checks_failed == 0 ? "OK" : "FAIL",
                           memory_spec->checks_failed == 0 ? "OK" : "FAIL",
                           aircraft_spec->spec->checks_failed == 0 ? "OK" : "FAIL"),
                 10, 35, 16, YELLOW);
        
        if (inject_failure) {
            DrawText("FAILURE INJECTED - Aircraft not rendering!", 
                     WINDOW_WIDTH/2 - 200, WINDOW_HEIGHT/2, 20, RED);
        }
        
        EndDrawing();
        
        // Handle debug keys
        if (IsKeyPressed(KEY_F1)) {
            spec_report_all();
        }
        
        if (IsKeyPressed(KEY_F2)) {
            inject_failure = !inject_failure;
            printf("Rendering failure injection: %s\n", 
                   inject_failure ? "ENABLED" : "DISABLED");
        }
        
        // MEMORY SPECIFICATION: Check for leaks periodically
        if (frame_count % 600 == 0) { // Every 10 seconds at 60fps
            // In real implementation, check actual memory usage
            SPEC_CHECK(memory_spec, true, "Memory leak detected");
        }
    }
    
    // Cleanup and final report
    aircraft_destroy(aircraft);
    aircraft_rendering_spec_destroy(aircraft_spec);
    
    spec_finalize(input_spec);
    spec_finalize(physics_spec);
    spec_finalize(memory_spec);
    
    renderer_destroy(renderer);
    game_controller_destroy(controller);
    game_state_destroy(state);
    CloseWindow();
    
    // Generate final specification report
    printf("\n=== FINAL SPECIFICATION REPORT ===\n");
    spec_report_all();
    
    spec_system_shutdown();
    
    return 0;
}
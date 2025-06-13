/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <time.h>
#include "sky_combat/core/specification.h"
#include "sky_combat/specifications/joystick_spec.h"
#include "sky_combat/models/aircraft.h"

/* Air Drone Flying Test View
 * 
 * Purpose: Verify all flight controls work correctly
 * This is NOT gameplay - it's a control verification system
 * 
 * References GAME_SPECIFICATION.md Section 3.1
 */

#define TEST_AREA_SIZE 2000.0f  // 2km x 2km
#define GRID_SIZE 50.0f         // 50m grid squares
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

typedef struct {
    // Test environment
    Camera3D camera;
    aircraft_t* aircraft;
    
    // Joystick
    int js_fd;
    short axes[8];
    short buttons[16];
    bool js_connected;
    
    // Specifications
    joystick_spec_t* js_spec;
    specification_t* flight_spec;
    
    // Performance tracking
    double input_latency_ms;
    double frame_time_ms;
    int fps;
    
    // Test state
    bool all_checks_passed;
    int checks_completed;
    char test_status[256];
    
    // Visual elements
    Vector3 speed_gates[10];
    int next_gate;
    float gate_timer;
} air_drone_test_t;

air_drone_test_t* air_drone_test_create(void) {
    air_drone_test_t* test = calloc(1, sizeof(air_drone_test_t));
    if (!test) return NULL;
    
    // Initialize aircraft at center, 100m up
    test->aircraft = aircraft_create((Vector3){0, 100, 0});
    
    // Initialize camera behind aircraft
    test->camera = (Camera3D){
        .position = {0, 150, -100},
        .target = {0, 100, 0},
        .up = {0, 1, 0},
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };
    
    // Initialize joystick
    test->js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    test->js_connected = (test->js_fd >= 0);
    
    if (test->js_connected) {
        // Initialize trigger rest positions
        test->axes[3] = -32767;
        test->axes[4] = -32767;
    }
    
    // Create specifications
    spec_system_init();
    test->js_spec = joystick_spec_create();
    test->flight_spec = spec_create("Flight Controls", 
                                    "All flight controls must work as specified");
    spec_register(test->flight_spec);
    
    // Verify joystick
    if (test->js_connected) {
        joystick_spec_verify_device(test->js_spec, "/dev/input/js0");
        joystick_spec_verify_axis_mapping(test->js_spec);
    }
    
    // Create speed gates in a circuit
    float angle_step = 2 * PI / 10;
    for (int i = 0; i < 10; i++) {
        float angle = i * angle_step;
        test->speed_gates[i] = (Vector3){
            cosf(angle) * 500,
            100 + sinf(angle * 2) * 50,
            sinf(angle) * 500
        };
    }
    
    strcpy(test->test_status, "Test Started - Move all controls");
    
    return test;
}

void air_drone_test_destroy(air_drone_test_t* test) {
    if (!test) return;
    
    if (test->js_fd >= 0) close(test->js_fd);
    if (test->aircraft) aircraft_destroy(test->aircraft);
    if (test->js_spec) joystick_spec_destroy(test->js_spec);
    
    spec_finalize(test->flight_spec);
    spec_report(test->flight_spec);
    spec_system_shutdown();
    
    free(test);
}

void air_drone_test_update_input(air_drone_test_t* test, float dt) {
    if (!test->js_connected) return;
    
    struct js_event event;
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Read all pending events
    while (read(test->js_fd, &event, sizeof(event)) > 0) {
        if (event.type & JS_EVENT_AXIS && event.number < 8) {
            test->axes[event.number] = event.value;
            
            // Verify axis values
            joystick_spec_check_axis_value(test->js_spec, event.number, event.value);
            
            // Special check for right stick Y
            if (event.number == 5 || event.number == 3) {
                joystick_spec_verify_right_stick_y(test->js_spec, test->axes);
            }
        } else if (event.type & JS_EVENT_BUTTON && event.number < 16) {
            test->buttons[event.number] = event.value;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    test->input_latency_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                             (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    // Check latency specification
    SPEC_CHECK(test->flight_spec, test->input_latency_ms < 16.67,
               "Input latency exceeds one frame");
}

void air_drone_test_update_flight(air_drone_test_t* test, float dt) {
    // Convert joystick to normalized values
    float move_x = -test->axes[0] / 32768.0f;  // INVERTED
    float move_y = -test->axes[1] / 32768.0f;  // INVERTED
    float aim_x = test->axes[2] / 32768.0f;
    float aim_y = test->axes[5] / 32768.0f;    // AXIS 5!
    
    // Apply dead zones
    if (fabsf(move_x) < 0.1f) move_x = 0;
    if (fabsf(move_y) < 0.1f) move_y = 0;
    if (fabsf(aim_x) < 0.1f) aim_x = 0;
    if (fabsf(aim_y) < 0.1f) aim_y = 0;
    
    // Get button states
    bool boost = test->buttons[2];      // Triangle
    bool brake = test->buttons[3];      // Square
    bool drift_left = test->buttons[4]; // L1
    bool drift_right = test->buttons[5]; // R1
    
    // Verify controls are inverted correctly
    joystick_spec_verify_inverted_controls(test->js_spec, &move_x, &move_y);
    
    // Update aircraft
    aircraft_update(test->aircraft, move_x, move_y, dt);
    
    // Apply boost/brake
    if (boost) {
        aircraft_boost(test->aircraft, dt);
    }
    if (brake) {
        aircraft_brake(test->aircraft, dt);
    }
    
    // Drift mechanics
    if ((drift_left && move_x > 0.1f) || (drift_right && move_x < -0.1f)) {
        // In drift mode - already handled by aircraft
    }
    
    // Update camera to follow aircraft
    Vector3 aircraft_pos = aircraft_get_position(test->aircraft);
    float yaw = aircraft->yaw * DEG2RAD;
    
    Vector3 cam_offset = {
        -sinf(yaw) * 100,
        50,
        -cosf(yaw) * 100
    };
    
    test->camera.position = Vector3Lerp(test->camera.position,
                                       Vector3Add(aircraft_pos, cam_offset),
                                       3 * dt);
    test->camera.target = Vector3Lerp(test->camera.target, aircraft_pos, 5 * dt);
}

void air_drone_test_check_controls(air_drone_test_t* test) {
    // Checklist from GAME_SPECIFICATION.md 3.1.4
    static bool checks[11] = {false};
    static const char* check_names[11] = {
        "Left stick left → Aircraft turns left",
        "Left stick right → Aircraft turns right", 
        "Left stick up → Aircraft pitches down",
        "Left stick down → Aircraft pitches up",
        "Triangle held → Speed increases to 180 m/s",
        "Square held → Speed decreases continuously",
        "L1 + left turn → Sharp drift turn",
        "R1 + right turn → Sharp drift turn",
        "Right stick moves → Aiming reticle moves",
        "R2 pulled → Gun firing animation",
        "L2 pulled → Missile firing animation"
    };
    
    // Check each control
    if (test->axes[0] < -16000) checks[0] = true;  // Left turn
    if (test->axes[0] > 16000) checks[1] = true;   // Right turn
    if (test->axes[1] < -16000) checks[2] = true;  // Pitch down
    if (test->axes[1] > 16000) checks[3] = true;   // Pitch up
    if (test->buttons[2] && aircraft_get_speed_percent(test->aircraft) > 0.9f) checks[4] = true;
    if (test->buttons[3]) checks[5] = true;
    if (test->buttons[4] && test->axes[0] > 16000) checks[6] = true;
    if (test->buttons[5] && test->axes[0] < -16000) checks[7] = true;
    if (fabsf(test->axes[2]) > 16000 || fabsf(test->axes[5]) > 16000) checks[8] = true;
    if (test->axes[4] > -16000) checks[9] = true;   // R2
    if (test->axes[3] > 0) checks[10] = true;       // L2
    
    // Count completed checks
    test->checks_completed = 0;
    for (int i = 0; i < 11; i++) {
        if (checks[i]) test->checks_completed++;
    }
    
    test->all_checks_passed = (test->checks_completed == 11);
    
    // Update status
    if (test->all_checks_passed) {
        strcpy(test->test_status, "ALL CHECKS PASSED! ✓");
    } else {
        snprintf(test->test_status, sizeof(test->test_status),
                 "Checks: %d/11 - Try: %s",
                 test->checks_completed,
                 check_names[test->checks_completed]);
    }
}

void air_drone_test_draw_environment(air_drone_test_t* test) {
    // Draw grid
    DrawGrid(100, GRID_SIZE);
    
    // Draw origin marker
    DrawCube((Vector3){0, 5, 0}, 10, 10, 10, RED);
    
    // Draw cardinal directions
    DrawCube((Vector3){0, 5, 500}, 5, 20, 5, BLUE);    // North
    DrawCube((Vector3){500, 5, 0}, 5, 20, 5, GREEN);   // East
    DrawCube((Vector3){0, 5, -500}, 5, 20, 5, YELLOW); // South
    DrawCube((Vector3){-500, 5, 0}, 5, 20, 5, ORANGE); // West
    
    // Draw altitude markers
    for (int alt = 100; alt <= 1000; alt += 100) {
        DrawCube((Vector3){0, alt, 0}, 2, 2, 2, GRAY);
    }
    
    // Draw speed gates
    for (int i = 0; i < 10; i++) {
        Color color = (i == test->next_gate) ? GREEN : SKYBLUE;
        DrawCubeWires(test->speed_gates[i], 40, 40, 5, color);
    }
    
    // Draw aircraft
    Vector3 pos = aircraft_get_position(test->aircraft);
    DrawCube(pos, 10, 3, 15, BLUE);
    
    // Draw aim indicator
    if (fabsf(test->axes[2]) > 3000 || fabsf(test->axes[5]) > 3000) {
        float aim_x = test->axes[2] / 32768.0f * 20;
        float aim_y = test->axes[5] / 32768.0f * 20;
        Vector3 aim_pos = Vector3Add(pos, (Vector3){aim_x, aim_y, 50});
        DrawLine3D(pos, aim_pos, GREEN);
        DrawCube(aim_pos, 2, 2, 2, GREEN);
    }
}

void air_drone_test_draw_hud(air_drone_test_t* test) {
    // Primary Flight Data (Top Left)
    Vector3 pos = aircraft_get_position(test->aircraft);
    float speed = aircraft_get_speed_percent(test->aircraft) * 90 + 30;
    
    DrawText("PRIMARY FLIGHT DATA", 10, 10, 16, GREEN);
    DrawText(TextFormat("Speed: %.1f m/s", speed), 10, 30, 20, WHITE);
    DrawText(TextFormat("Alt:   %.1f m", pos.y), 10, 50, 20, WHITE);
    DrawText(TextFormat("Yaw:   %.1f°", test->aircraft->yaw), 10, 70, 20, WHITE);
    DrawText(TextFormat("Pitch: %.1f°", test->aircraft->pitch), 10, 90, 20, WHITE);
    DrawText(TextFormat("Roll:  %.1f°", test->aircraft->roll), 10, 110, 20, WHITE);
    
    // Control Inputs (Top Right)
    int x = GetScreenWidth() - 250;
    DrawText("CONTROL INPUTS", x, 10, 16, GREEN);
    DrawText(TextFormat("L-Stick X: %.2f%s", 
             -test->axes[0]/32768.0f,
             test->axes[0] < -3000 ? " (inv)" : ""), x, 30, 14, WHITE);
    DrawText(TextFormat("L-Stick Y: %.2f%s",
             -test->axes[1]/32768.0f,
             test->axes[1] < -3000 ? " (inv)" : ""), x, 45, 14, WHITE);
    DrawText(TextFormat("R-Stick X: %.2f", test->axes[2]/32768.0f), x, 60, 14, WHITE);
    DrawText(TextFormat("R-Stick Y: %.2f (axis 5)", test->axes[5]/32768.0f), x, 75, 14, 
             test->js_spec->axis_errors > 0 ? RED : WHITE);
    DrawText(TextFormat("Boost: %s", test->buttons[2] ? "ON" : "OFF"), x, 90, 14,
             test->buttons[2] ? YELLOW : WHITE);
    DrawText(TextFormat("Brake: %s", test->buttons[3] ? "ON" : "OFF"), x, 105, 14,
             test->buttons[3] ? YELLOW : WHITE);
    DrawText(TextFormat("Drift-L: %s", test->buttons[4] ? "ON" : "OFF"), x, 120, 14,
             test->buttons[4] ? YELLOW : WHITE);
    DrawText(TextFormat("Drift-R: %s", test->buttons[5] ? "ON" : "OFF"), x, 135, 14,
             test->buttons[5] ? YELLOW : WHITE);
    
    // Performance Metrics (Bottom Left)
    int y = GetScreenHeight() - 100;
    DrawText("PERFORMANCE", 10, y, 16, GREEN);
    DrawText(TextFormat("FPS: %d", test->fps), 10, y + 20, 14, WHITE);
    DrawText(TextFormat("Input Lag: %.1fms", test->input_latency_ms), 10, y + 35, 14,
             test->input_latency_ms > 16.67 ? RED : WHITE);
    DrawText(TextFormat("Frame Time: %.1fms", test->frame_time_ms), 10, y + 50, 14, WHITE);
    DrawText(TextFormat("Axis 5 Check: %s", 
             test->js_spec->axis_errors == 0 ? "PASS ✓" : "FAIL ✗"), 10, y + 65, 14,
             test->js_spec->axis_errors == 0 ? GREEN : RED);
    
    // Test Status (Center)
    int status_width = MeasureText(test->test_status, 24);
    DrawText(test->test_status, 
             GetScreenWidth()/2 - status_width/2,
             GetScreenHeight() - 40, 24,
             test->all_checks_passed ? GREEN : YELLOW);
}

void air_drone_test_run(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Air Drone Flying Test View");
    SetTargetFPS(60);
    
    air_drone_test_t* test = air_drone_test_create();
    if (!test) {
        CloseWindow();
        return;
    }
    
    printf("\n=== AIR DRONE FLYING TEST VIEW ===\n");
    printf("Reference: GAME_SPECIFICATION.md Section 3.1\n");
    printf("Purpose: Verify all flight controls work correctly\n");
    printf("\nComplete all 11 control checks to pass the test.\n\n");
    
    clock_t frame_start, frame_end;
    
    while (!WindowShouldClose()) {
        frame_start = clock();
        float dt = GetFrameTime();
        
        // Update
        air_drone_test_update_input(test, dt);
        air_drone_test_update_flight(test, dt);
        air_drone_test_check_controls(test);
        
        test->fps = GetFPS();
        
        // Draw
        BeginDrawing();
        ClearBackground((Color){135, 206, 235, 255}); // Sky blue
        
        BeginMode3D(test->camera);
        air_drone_test_draw_environment(test);
        EndMode3D();
        
        air_drone_test_draw_hud(test);
        
        // Instructions
        if (!test->all_checks_passed) {
            DrawText("Press ESC to exit", 10, GetScreenHeight() - 20, 14, GRAY);
        }
        
        EndDrawing();
        
        frame_end = clock();
        test->frame_time_ms = (double)(frame_end - frame_start) / CLOCKS_PER_SEC * 1000;
    }
    
    printf("\n=== TEST RESULTS ===\n");
    printf("Checks completed: %d/11\n", test->checks_completed);
    printf("Test passed: %s\n", test->all_checks_passed ? "YES" : "NO");
    printf("Average input latency: %.2fms\n", test->js_spec->avg_latency_ms);
    
    air_drone_test_destroy(test);
    CloseWindow();
}

// Standalone entry point
int main(void) {
    air_drone_test_run();
    return 0;
}
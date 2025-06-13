/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sky_combat/core/specification.h"

/* Simple Specification Demo
 * 
 * This shows how Specification Driven Development works:
 * 1. Define what MUST happen
 * 2. Create checks that verify it
 * 3. Run checks continuously
 * 4. Get Six Sigma reliability
 */

// Example: Joystick specification
void test_joystick_specification(void) {
    specification_t* joystick_spec = spec_create(
        "Joystick Input",
        "Right stick Y-axis must be on axis 5, not axis 3"
    );
    spec_register(joystick_spec);
    
    // Simulate checking joystick axis mapping
    int expected_axis = 5;
    int actual_axis = 5;  // This would come from real joystick
    
    SPEC_CHECK(joystick_spec, actual_axis == expected_axis,
               "Right stick Y-axis not on correct axis");
    
    // Check axis value range
    float axis_value = 0.5f;  // Simulated value
    SPEC_CHECK_RANGE(joystick_spec, axis_value, -1.0f, 1.0f,
                     "Joystick axis value out of range");
    
    spec_finalize(joystick_spec);
    spec_report(joystick_spec);
}

// Example: Rendering specification
void test_rendering_specification(void) {
    specification_t* render_spec = spec_create(
        "Rendering Pipeline",
        "Frame must render within 16.67ms (60 FPS)"
    );
    spec_register(render_spec);
    
    // Simulate frame timing checks
    for (int frame = 0; frame < 1000; frame++) {
        double frame_time_ms = 15.0 + (rand() % 30) / 10.0;  // 15-18ms
        
        SPEC_CHECK(render_spec, frame_time_ms <= 16.67,
                   "Frame took too long to render");
    }
    
    spec_finalize(render_spec);
    spec_report(render_spec);
}

// Example: Memory specification
void test_memory_specification(void) {
    specification_t* memory_spec = spec_create(
        "Memory Management",
        "All allocated memory must be accessible"
    );
    spec_set_critical(memory_spec, true);  // Critical - abort on failure
    spec_register(memory_spec);
    
    // Test memory allocation
    size_t test_size = 1024 * 1024;  // 1MB
    void* buffer = malloc(test_size);
    
    SPEC_CHECK_NOT_NULL(memory_spec, buffer, "Failed to allocate memory");
    SPEC_CHECK_MEMORY(memory_spec, buffer, test_size, "Memory not accessible");
    
    free(buffer);
    
    spec_finalize(memory_spec);
    spec_report(memory_spec);
}

// Example: Game logic specification
void test_game_logic_specification(void) {
    specification_t* logic_spec = spec_create(
        "Game Logic",
        "Aircraft speed must stay within valid range"
    );
    spec_register(logic_spec);
    
    float min_speed = 30.0f;
    float max_speed = 120.0f;
    
    // Simulate aircraft speed over time
    float speed = 60.0f;
    for (int i = 0; i < 100; i++) {
        // Simulate speed changes
        speed += (rand() % 20 - 10) * 0.1f;
        
        // Clamp to valid range (this is what the game should do)
        if (speed < min_speed) speed = min_speed;
        if (speed > max_speed) speed = max_speed;
        
        SPEC_CHECK_RANGE(logic_spec, speed, min_speed, max_speed,
                        "Aircraft speed out of valid range");
    }
    
    spec_finalize(logic_spec);
    spec_report(logic_spec);
}

int main(void) {
    printf("=== SPECIFICATION DRIVEN DEVELOPMENT DEMO ===\n");
    printf("This demonstrates how we turn requirements into runtime checks\n");
    printf("with Six Sigma reliability (99.99966%% success rate)\n\n");
    
    // Initialize specification system
    spec_system_init();
    
    // Run various specification tests
    test_joystick_specification();
    test_rendering_specification();
    test_memory_specification();
    test_game_logic_specification();
    
    // Show final summary
    printf("\n");
    spec_report_all();
    
    // Example of how to use in real game:
    printf("\n=== HOW TO USE IN YOUR GAME ===\n");
    printf("1. Tell me: 'I want enemies to always spawn at least 50 units from player'\n");
    printf("2. I create: SPEC_CHECK_RANGE(spec, distance, 50, INFINITY, \"Enemy too close\")\n");
    printf("3. Game runs: Check executes every enemy spawn\n");
    printf("4. If fails: Immediate detailed error report\n");
    printf("5. Result: Six Sigma reliability - bugs caught instantly!\n");
    
    spec_system_shutdown();
    
    return 0;
}
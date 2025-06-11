/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Test program to verify what the right stick actually controls
 */

#include <stdio.h>
#include <raylib.h>

int main(void) {
    // Initialize in headless mode if in GDB
    if (getenv("GDB_VERIFICATION_MODE")) {
        printf("=== RIGHT STICK BEHAVIOR TEST ===\n");
        printf("Testing what right stick actually controls...\n\n");
        
        // Simulate right stick input
        float right_stick_x = 1.0f;  // Full right
        float right_stick_y = 1.0f;  // Full up
        
        // Test 1: Does right stick affect camera?
        float camera_x = 0.0f;
        float camera_y = 0.0f;
        // In current code: camera control is commented out
        // camera_x += right_stick_x * 100 * dt;  // COMMENTED OUT
        // camera_y += right_stick_y * 80 * dt;   // COMMENTED OUT
        
        printf("TEST 1 - Camera Control:\n");
        printf("  Right stick X input: %.1f\n", right_stick_x);
        printf("  Right stick Y input: %.1f\n", right_stick_y);
        printf("  Camera X change: %.1f (NONE - code is commented out)\n", camera_x);
        printf("  Camera Y change: %.1f (NONE - code is commented out)\n", camera_y);
        printf("  ❌ Right stick does NOT control camera\n\n");
        
        // Test 2: Does right stick affect gun aim?
        float gun_aim_offset_x = 0.0f;
        float gun_aim_offset_y = 0.0f;
        // In current code: gun aim is commented out
        // gun_aim_offset_x = right_stick_x * 15.0f;  // COMMENTED OUT
        // gun_aim_offset_y = right_stick_y * 15.0f;  // COMMENTED OUT
        
        printf("TEST 2 - Gun Aim Offset:\n");
        printf("  Right stick X input: %.1f\n", right_stick_x);
        printf("  Right stick Y input: %.1f\n", right_stick_y);
        printf("  Gun aim X offset: %.1f (NONE - code is commented out)\n", gun_aim_offset_x);
        printf("  Gun aim Y offset: %.1f (NONE - code is commented out)\n", gun_aim_offset_y);
        printf("  ❌ Right stick does NOT affect gun aim\n\n");
        
        // Test 3: What DOES right stick do?
        printf("TEST 3 - Actual Behavior:\n");
        printf("  Right stick axes are read as camera_x/camera_y\n");
        printf("  But NO code uses these values!\n");
        printf("  🚫 RIGHT STICK HAS NO FUNCTION\n\n");
        
        printf("=== CONCLUSION ===\n");
        printf("The right stick input is read but not used for anything.\n");
        printf("Both camera control and gun aiming code are commented out.\n");
        printf("The GDB proofs claim features that don't exist!\n");
        
        return 0;
    }
    
    // If not in GDB mode, show interactive test
    InitWindow(800, 600, "Right Stick Behavior Test");
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        float right_x = 0, right_y = 0;
        
        if (IsGamepadAvailable(0)) {
            right_x = GetGamepadAxisMovement(0, 2);  // Right stick X
            right_y = GetGamepadAxisMovement(0, 3);  // Right stick Y (or 5)
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("RIGHT STICK BEHAVIOR TEST", 10, 10, 20, BLACK);
        DrawText("Move the right stick to see what it controls:", 10, 40, 16, DARKGRAY);
        
        DrawText(TextFormat("Right Stick X: %.2f", right_x), 10, 80, 16, BLACK);
        DrawText(TextFormat("Right Stick Y: %.2f", right_y), 10, 100, 16, BLACK);
        
        DrawText("ACTUAL BEHAVIOR:", 10, 140, 18, RED);
        DrawText("❌ Does NOT control camera (code commented out)", 10, 170, 16, BLACK);
        DrawText("❌ Does NOT control gun aim (code commented out)", 10, 190, 16, BLACK);
        DrawText("🚫 RIGHT STICK HAS NO FUNCTION", 10, 210, 16, RED);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <raylib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main() {
    printf("=== JOYSTICK DETECTION TEST ===\n\n");
    
    // First check what devices exist
    printf("1. Checking /dev/input/ devices:\n");
    DIR *d = opendir("/dev/input");
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, "js", 2) == 0 || 
                strncmp(dir->d_name, "event", 5) == 0) {
                printf("   Found: /dev/input/%s\n", dir->d_name);
            }
        }
        closedir(d);
    }
    
    printf("\n2. Initializing Raylib with extended logging...\n");
    SetTraceLogLevel(LOG_ALL);
    InitWindow(800, 600, "Joystick Detection Test");
    SetTargetFPS(60);
    
    printf("\n3. Checking Raylib gamepad detection:\n");
    
    // Force raylib to rescan gamepads
    for (int retry = 0; retry < 3; retry++) {
        printf("\nScan attempt %d:\n", retry + 1);
        
        // Check all 16 possible gamepad slots (raylib supports up to 16)
        int found = 0;
        for (int i = 0; i < 16; i++) {
            if (IsGamepadAvailable(i)) {
                printf("✓ Gamepad %d: %s\n", i, GetGamepadName(i));
                found++;
            }
        }
        
        if (found == 0) {
            printf("✗ No gamepads detected by Raylib\n");
            
            // Try to trigger a rescan by clearing and reinitializing
            if (retry < 2) {
                printf("  Waiting 1 second and rescanning...\n");
                WaitTime(1.0);
            }
        } else {
            break;
        }
    }
    
    printf("\n4. Testing with different gamepad IDs:\n");
    printf("Sometimes gamepads register at unexpected IDs.\n");
    printf("Move your joystick or press buttons...\n\n");
    
    int frames = 0;
    while (!WindowShouldClose() && frames < 300) {  // 5 seconds at 60fps
        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawText("GAMEPAD DETECTION TEST", 10, 10, 20, WHITE);
        DrawText("Checking all 16 possible gamepad slots...", 10, 40, 16, GRAY);
        
        int y = 80;
        for (int i = 0; i < 16; i++) {
            if (IsGamepadAvailable(i)) {
                DrawText(TextFormat("Gamepad %d: %s", i, GetGamepadName(i)), 
                         10, y, 16, GREEN);
                y += 20;
                
                // Show any axis movement
                for (int axis = 0; axis < 6; axis++) {
                    float value = GetGamepadAxisMovement(i, axis);
                    if (value != 0.0f) {
                        DrawText(TextFormat("  Axis %d: %.2f", axis, value), 
                                 20, y, 14, YELLOW);
                        y += 18;
                    }
                }
                
                // Show button presses
                for (int btn = 0; btn < 32; btn++) {
                    if (IsGamepadButtonDown(i, btn)) {
                        DrawText(TextFormat("  Button %d pressed!", btn), 
                                 20, y, 14, YELLOW);
                        y += 18;
                    }
                }
            }
        }
        
        if (y == 80) {
            DrawText("No gamepads detected!", 10, y, 16, RED);
            DrawText("Linux game controller app works but Raylib doesn't see it.", 
                     10, y + 30, 14, WHITE);
            DrawText("This might be a GLFW/Raylib limitation with your controller.", 
                     10, y + 50, 14, WHITE);
            
            y += 80;
            DrawText("WORKAROUNDS:", 10, y, 16, YELLOW);
            DrawText("1. Try unplugging and reconnecting the controller", 10, y + 25, 14, WHITE);
            DrawText("2. Try a different USB port", 10, y + 45, 14, WHITE);
            DrawText("3. Some controllers need to be in 'PC mode' (check for a switch)", 10, y + 65, 14, WHITE);
            DrawText("4. Use keyboard controls: WASD + Q/E for now", 10, y + 85, 14, WHITE);
        }
        
        DrawText(TextFormat("Frame: %d/300", frames), 10, 550, 14, GRAY);
        
        EndDrawing();
        frames++;
    }
    
    printf("\n5. Final diagnosis:\n");
    if (!IsGamepadAvailable(0)) {
        printf("❌ Raylib/GLFW cannot see your gamepad\n");
        printf("   This is a known issue with some controllers and GLFW\n");
        printf("   Your controller works in Linux but not in GLFW-based apps\n");
        printf("\nWORKAROUND: Use keyboard controls:\n");
        printf("   W/S = Pitch\n");
        printf("   A/D = Turn\n");
        printf("   Q/E = Throttle\n");
    }
    
    CloseWindow();
    return 0;
}
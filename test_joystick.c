/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <raylib.h>
#include <stdio.h>

int main() {
    InitWindow(800, 600, "Joystick Test");
    SetTargetFPS(60);
    
    printf("=== JOYSTICK DIAGNOSTIC ===\n\n");
    
    // Check all possible gamepads
    printf("Scanning for gamepads...\n");
    int active_gamepad = -1;
    
    for (int i = 0; i < 4; i++) {
        if (IsGamepadAvailable(i)) {
            printf("✓ Gamepad %d: %s\n", i, GetGamepadName(i));
            if (active_gamepad == -1) active_gamepad = i;
        } else {
            printf("✗ Gamepad %d: Not connected\n", i);
        }
    }
    
    if (active_gamepad == -1) {
        printf("\nNO GAMEPAD DETECTED!\n");
        printf("Possible issues:\n");
        printf("1. Joystick not properly connected\n");
        printf("2. Need to run: sudo chmod 666 /dev/input/js0\n");
        printf("3. May need to install: sudo zypper install joystick\n");
    }
    
    printf("\nPress ESC to exit\n\n");
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawText("JOYSTICK DIAGNOSTIC", 10, 10, 20, WHITE);
        
        int y = 50;
        for (int i = 0; i < 4; i++) {
            if (IsGamepadAvailable(i)) {
                DrawText(TextFormat("Gamepad %d: %s", i, GetGamepadName(i)), 10, y, 16, GREEN);
                y += 25;
                
                // Show all axes
                for (int axis = 0; axis < 6; axis++) {
                    float value = GetGamepadAxisMovement(i, axis);
                    const char* axis_names[] = {"LEFT_X", "LEFT_Y", "RIGHT_X", "RIGHT_Y", "L_TRIGGER", "R_TRIGGER"};
                    
                    DrawText(TextFormat("  %s: %.2f", axis_names[axis], value), 20, y, 14, WHITE);
                    
                    // Draw bar
                    DrawRectangle(200, y, 200, 15, DARKGRAY);
                    DrawRectangle(200 + 100, y, (int)(value * 100), 15, GREEN);
                    y += 20;
                }
                
                // Show buttons
                DrawText("  Buttons:", 20, y, 14, WHITE);
                for (int btn = 0; btn < 16; btn++) {
                    if (IsGamepadButtonDown(i, btn)) {
                        DrawText(TextFormat("%d", btn), 100 + btn * 20, y, 14, YELLOW);
                    } else {
                        DrawText(TextFormat("%d", btn), 100 + btn * 20, y, 14, DARKGRAY);
                    }
                }
                y += 30;
            } else {
                DrawText(TextFormat("Gamepad %d: Not connected", i), 10, y, 16, RED);
                y += 25;
            }
        }
        
        // Show raw device info
        DrawText("Raw device: /dev/input/js0", 10, 500, 14, GRAY);
        DrawText("If no gamepad detected, try:", 10, 520, 14, GRAY);
        DrawText("  sudo chmod 666 /dev/input/js0", 10, 540, 14, YELLOW);
        DrawText("  Then restart this program", 10, 560, 14, YELLOW);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
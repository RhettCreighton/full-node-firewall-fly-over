/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "sky_combat/core/game_state.h"
#include "sky_combat/commands/command_system.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// Command mode toggle
static bool command_mode = false;
static char command_buffer[256] = {0};
static int command_cursor = 0;

void draw_help_overlay(void) {
    DrawRectangle(10, 10, 400, 200, Fade(BLACK, 0.8f));
    DrawRectangleLines(10, 10, 400, 200, GREEN);
    
    DrawText("SKY COMBAT MODULAR - Command System", 20, 20, 20, GREEN);
    DrawText("Press ~ to open command console", 20, 50, 16, WHITE);
    DrawText("Press F1 for this help", 20, 70, 16, WHITE);
    DrawText("Press F2 for debug mode", 20, 90, 16, WHITE);
    DrawText("Press F3 for stats", 20, 110, 16, WHITE);
    
    DrawText("Common Commands:", 20, 140, 16, YELLOW);
    DrawText("  test flight - Test flight controls", 20, 160, 16, WHITE);
    DrawText("  test world - Test world generation", 20, 180, 16, WHITE);
}

void draw_command_console(command_system_t* cmd_sys) {
    // Draw console background
    DrawRectangle(0, GetScreenHeight() - 300, GetScreenWidth(), 300, Fade(BLACK, 0.9f));
    DrawRectangleLines(0, GetScreenHeight() - 300, GetScreenWidth(), 300, GREEN);
    
    // Draw title
    DrawText("COMMAND CONSOLE", 10, GetScreenHeight() - 290, 20, GREEN);
    DrawText("Type 'help' for commands", 10, GetScreenHeight() - 265, 16, GRAY);
    
    // Draw command history
    int y = GetScreenHeight() - 240;
    for (int i = 0; i < cmd_sys->history_count && i < 10; i++) {
        int idx = cmd_sys->history_count - 1 - i;
        DrawText(cmd_sys->history[idx], 10, y + i * 20, 16, DARKGRAY);
    }
    
    // Draw input line
    DrawText("> ", 10, GetScreenHeight() - 30, 20, GREEN);
    DrawText(command_buffer, 30, GetScreenHeight() - 30, 20, WHITE);
    
    // Draw cursor
    if ((int)(GetTime() * 2) % 2) {
        int cursor_x = 30 + MeasureText(command_buffer, 20);
        DrawRectangle(cursor_x, GetScreenHeight() - 30, 10, 20, GREEN);
    }
}

void handle_command_input(command_system_t* cmd_sys, game_state_t* state) {
    if (IsKeyPressed(KEY_ENTER)) {
        if (strlen(command_buffer) > 0) {
            // Execute command
            command_execute(cmd_sys, state, command_buffer);
            
            // Clear buffer
            memset(command_buffer, 0, sizeof(command_buffer));
            command_cursor = 0;
        }
    } else if (IsKeyPressed(KEY_BACKSPACE)) {
        if (command_cursor > 0) {
            command_cursor--;
            command_buffer[command_cursor] = '\0';
        }
    } else {
        // Handle text input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && command_cursor < 255) {
                command_buffer[command_cursor] = (char)key;
                command_cursor++;
                command_buffer[command_cursor] = '\0';
            }
            key = GetCharPressed();
        }
    }
}

int main(int argc, char** argv) {
    // Initialize window
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sky Combat Modular - MVC Architecture");
    SetTargetFPS(60);
    
    // Create game state
    game_state_t* state = game_state_create();
    if (!state) {
        printf("Failed to create game state!\n");
        return 1;
    }
    
    // Get command system
    command_system_t* cmd_sys = state->commands;
    
    // Execute startup commands if provided
    if (argc > 1) {
        if (strcmp(argv[1], "-c") == 0 && argc > 2) {
            command_execute(cmd_sys, state, argv[2]);
        } else if (strcmp(argv[1], "-f") == 0 && argc > 2) {
            command_execute_file(cmd_sys, state, argv[2]);
        }
    }
    
    // Show initial help
    printf("\n=== SKY COMBAT MODULAR ===\n");
    printf("Press ~ to open command console\n");
    printf("Press F1 for help overlay\n");
    printf("Type 'help' in console for all commands\n\n");
    
    // Main game loop
    while (!WindowShouldClose() && state->running) {
        float dt = GetFrameTime();
        
        // Toggle command mode
        if (IsKeyPressed(KEY_GRAVE)) {
            command_mode = !command_mode;
            if (!command_mode) {
                // Clear command buffer when closing
                memset(command_buffer, 0, sizeof(command_buffer));
                command_cursor = 0;
            }
        }
        
        // Handle input based on mode
        if (command_mode) {
            handle_command_input(cmd_sys, state);
        } else {
            // Normal game input
            game_state_handle_input(state);
            
            // Debug shortcuts
            if (IsKeyPressed(KEY_F2)) {
                state->debug_mode = !state->debug_mode;
            }
            if (IsKeyPressed(KEY_F3)) {
                state->show_stats = !state->show_stats;
            }
        }
        
        // Update game state
        if (!state->paused || IsKeyPressed(KEY_F4)) {  // F4 = frame step
            game_state_update(state, dt * state->time_scale);
        }
        
        // Draw everything
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw game
        game_state_draw(state);
        
        // Draw UI overlays
        if (IsKeyDown(KEY_F1)) {
            draw_help_overlay();
        }
        
        if (command_mode) {
            draw_command_console(cmd_sys);
        }
        
        // Debug info
        if (state->debug_mode) {
            DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
            DrawText(TextFormat("Mode: %d", state->mode), 10, 35, 20, GREEN);
            DrawText(TextFormat("Time: %.1f", state->total_time), 10, 60, 20, GREEN);
        }
        
        EndDrawing();
    }
    
    // Cleanup
    game_state_destroy(state);
    CloseWindow();
    
    return 0;
}

/* Example usage:
 * 
 * ./sky_combat_main                    # Start normally
 * ./sky_combat_main -c "test flight"   # Start with flight test
 * ./sky_combat_main -f commands.txt    # Execute commands from file
 * 
 * In-game commands:
 *   help                    - Show all commands
 *   test flight             - Test flight controls
 *   test world              - Test world generation
 *   test building interior  - Test enterable buildings
 *   test characters         - Test ground NPCs
 *   spawn enemy 10          - Spawn 10 enemies
 *   teleport 0 100 0        - Teleport to position
 *   set speed 200           - Set aircraft speed
 *   toggle debug            - Toggle debug mode
 *   quit                    - Exit game
 */
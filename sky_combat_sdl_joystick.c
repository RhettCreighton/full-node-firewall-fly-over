/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Joystick* joystick = NULL;

typedef struct {
    float x, y, z;
    float pitch, yaw, roll;
    float throttle;
} Aircraft;

Aircraft player = {0, 200, 0, 0, 0, 0, 50};

int init_sdl_joystick() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return -1;
    }
    
    window = SDL_CreateWindow("Sky Combat - SDL Joystick",
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // Check for joysticks
    int num_joysticks = SDL_NumJoysticks();
    printf("\n=== SDL JOYSTICK DETECTION ===\n");
    printf("Found %d joystick(s)\n", num_joysticks);
    
    if (num_joysticks > 0) {
        joystick = SDL_JoystickOpen(0);
        if (joystick) {
            printf("✓ Opened joystick: %s\n", SDL_JoystickName(joystick));
            printf("  Axes: %d\n", SDL_JoystickNumAxes(joystick));
            printf("  Buttons: %d\n", SDL_JoystickNumButtons(joystick));
            printf("  Hats: %d\n", SDL_JoystickNumHats(joystick));
        } else {
            printf("✗ Failed to open joystick: %s\n", SDL_GetError());
        }
    }
    
    return 0;
}

void update_with_joystick(float dt) {
    if (joystick) {
        // Get axis values (-32768 to 32767)
        Sint16 axis_x = SDL_JoystickGetAxis(joystick, 0);  // Left stick X
        Sint16 axis_y = SDL_JoystickGetAxis(joystick, 1);  // Left stick Y
        Sint16 axis_rx = SDL_JoystickGetAxis(joystick, 3); // Right stick X
        Sint16 axis_ry = SDL_JoystickGetAxis(joystick, 4); // Right stick Y
        
        // Convert to float (-1 to 1)
        float stick_x = axis_x / 32768.0f;
        float stick_y = axis_y / 32768.0f;
        float rstick_x = axis_rx / 32768.0f;
        float rstick_y = axis_ry / 32768.0f;
        
        // Use right stick for flight control
        player.pitch -= rstick_y * 60 * dt;
        player.yaw += rstick_x * 90 * dt;
        player.roll = -rstick_x * 30;
        
        // Triggers for throttle (if available)
        Sint16 rtrigger = SDL_JoystickGetAxis(joystick, 5);
        if (rtrigger > 0) {
            player.throttle += (rtrigger / 32768.0f) * 100 * dt;
        }
        
        // Buttons
        if (SDL_JoystickGetButton(joystick, 0)) {  // A/X button
            player.throttle += 100 * dt;
        }
        if (SDL_JoystickGetButton(joystick, 1)) {  // B/Circle button
            player.throttle -= 100 * dt;
        }
    }
    
    // Keyboard fallback
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_W]) player.pitch -= 60 * dt;
    if (keys[SDL_SCANCODE_S]) player.pitch += 60 * dt;
    if (keys[SDL_SCANCODE_A]) player.yaw -= 90 * dt;
    if (keys[SDL_SCANCODE_D]) player.yaw += 90 * dt;
    if (keys[SDL_SCANCODE_Q]) player.throttle -= 100 * dt;
    if (keys[SDL_SCANCODE_E]) player.throttle += 100 * dt;
    
    // Clamp values
    if (player.throttle < 0) player.throttle = 0;
    if (player.throttle > 200) player.throttle = 200;
    if (player.pitch < -80) player.pitch = -80;
    if (player.pitch > 80) player.pitch = 80;
}

void draw_scene() {
    SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
    SDL_RenderClear(renderer);
    
    // Draw simple horizon line based on pitch
    SDL_SetRenderDrawColor(renderer, 0, 100, 200, 255);
    int horizon_y = SCREEN_HEIGHT/2 - (int)(player.pitch * 3);
    SDL_RenderDrawLine(renderer, 0, horizon_y, SCREEN_WIDTH, horizon_y);
    
    // HUD
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    
    // Draw text would go here (SDL_ttf needed for real text)
    // For now, draw indicators
    
    // Altitude bar
    SDL_Rect alt_bar = {20, 50, 30, 300};
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
    SDL_RenderFillRect(renderer, &alt_bar);
    
    SDL_Rect alt_level = {20, 350 - (int)(player.y), 30, (int)(player.y)};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &alt_level);
    
    // Throttle bar
    SDL_Rect thr_bar = {80, 50, 30, 300};
    SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
    SDL_RenderFillRect(renderer, &thr_bar);
    
    SDL_Rect thr_level = {80, 350 - (int)(player.throttle * 1.5f), 30, (int)(player.throttle * 1.5f)};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &thr_level);
    
    // Crosshair
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(renderer, SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT/2, 
                                SCREEN_WIDTH/2 - 10, SCREEN_HEIGHT/2);
    SDL_RenderDrawLine(renderer, SCREEN_WIDTH/2 + 10, SCREEN_HEIGHT/2, 
                                SCREEN_WIDTH/2 + 30, SCREEN_HEIGHT/2);
    SDL_RenderDrawLine(renderer, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 30, 
                                SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 10);
    SDL_RenderDrawLine(renderer, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 10, 
                                SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 30);
    
    // Joystick status
    if (joystick) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect js_indicator = {SCREEN_WIDTH - 150, 20, 130, 20};
        SDL_RenderFillRect(renderer, &js_indicator);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect js_indicator = {SCREEN_WIDTH - 150, 20, 130, 20};
        SDL_RenderFillRect(renderer, &js_indicator);
    }
    
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (init_sdl_joystick() < 0) {
        return 1;
    }
    
    printf("\nCONTROLS:\n");
    if (joystick) {
        printf("JOYSTICK:\n");
        printf("  Right Stick - Fly\n");
        printf("  A/X Button - Throttle up\n");
        printf("  B/Circle - Throttle down\n");
    }
    printf("KEYBOARD:\n");
    printf("  WASD - Fly\n");
    printf("  Q/E - Throttle\n");
    printf("  ESC - Exit\n");
    
    SDL_Event event;
    int running = 1;
    Uint32 last_time = SDL_GetTicks();
    
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
        }
        
        update_with_joystick(dt);
        
        // Simple physics
        float yaw_rad = player.yaw * M_PI / 180.0f;
        float pitch_rad = player.pitch * M_PI / 180.0f;
        
        player.x += player.throttle * cosf(yaw_rad) * cosf(pitch_rad) * dt;
        player.y += player.throttle * sinf(pitch_rad) * dt;
        player.z += player.throttle * sinf(yaw_rad) * cosf(pitch_rad) * dt;
        
        if (player.y < 5) player.y = 5;
        
        draw_scene();
        
        SDL_Delay(16);  // ~60 FPS
    }
    
    if (joystick) {
        SDL_JoystickClose(joystick);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
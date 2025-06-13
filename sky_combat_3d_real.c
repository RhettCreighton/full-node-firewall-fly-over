/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BUILDING_COUNT 100
#define AI_COUNT 200

typedef struct {
    Vector3 pos;
    Vector3 size;
    Color color;
    bool has_neon;
} Building;

typedef struct {
    Vector3 pos;
    Color color;
    bool fleeing;
} AIChar;

Building buildings[BUILDING_COUNT];
AIChar ai_chars[AI_COUNT];
Camera camera = { 0 };

int main() {
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat 3D - Cyberpunk City");
    SetTargetFPS(60);
    
    // Initialize camera
    camera.position = (Vector3){ 0.0f, 100.0f, -200.0f };
    camera.target = (Vector3){ 0.0f, 50.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Camera will be controlled with UpdateCamera
    
    // Generate city
    for (int i = 0; i < BUILDING_COUNT; i++) {
        buildings[i].pos.x = GetRandomValue(-1000, 1000);
        buildings[i].pos.z = GetRandomValue(-1000, 1000);
        buildings[i].pos.y = 0;
        
        buildings[i].size.x = GetRandomValue(30, 80);
        buildings[i].size.z = GetRandomValue(30, 80);
        buildings[i].size.y = GetRandomValue(50, 400);
        
        buildings[i].color = (Color){
            GetRandomValue(20, 40),
            GetRandomValue(20, 40),
            GetRandomValue(30, 60),
            255
        };
        
        buildings[i].has_neon = GetRandomValue(0, 100) < 60;
    }
    
    // Spawn AI
    for (int i = 0; i < AI_COUNT; i++) {
        ai_chars[i].pos.x = GetRandomValue(-1000, 1000);
        ai_chars[i].pos.z = GetRandomValue(-1000, 1000);
        ai_chars[i].pos.y = 1.0f;
        ai_chars[i].color = ORANGE;
        ai_chars[i].fleeing = false;
    }
    
    printf("\n=== SKY COMBAT 3D WORLD - RUNNING ===\n");
    printf("This is the REAL 3D world you wanted!\n");
    printf("Controls:\n");
    printf("  Mouse - Look around\n");
    printf("  WASD - Move\n");
    printf("  Shift - Move faster\n");
    printf("  ESC - Exit\n\n");
    
    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        UpdateCamera(&camera, CAMERA_FREE);
        
        // Check AI fleeing
        for (int i = 0; i < AI_COUNT; i++) {
            float dist = Vector3Distance(camera.position, ai_chars[i].pos);
            ai_chars[i].fleeing = (camera.position.y < 50 && dist < 50);
            if (ai_chars[i].fleeing) {
                ai_chars[i].color = RED;
            } else {
                ai_chars[i].color = ORANGE;
            }
        }
        
        // Draw
        BeginDrawing();
        
        ClearBackground((Color){10, 10, 30, 255}); // Night sky
        
        BeginMode3D(camera);
        
        // Draw ground
        DrawPlane((Vector3){0, 0, 0}, (Vector2){2000, 2000}, DARKGRAY);
        
        // Draw buildings
        for (int i = 0; i < BUILDING_COUNT; i++) {
            Vector3 pos = buildings[i].pos;
            pos.y = buildings[i].size.y / 2;
            
            DrawCube(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, buildings[i].color);
            DrawCubeWires(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, BLACK);
            
            // Neon on top
            if (buildings[i].has_neon) {
                Vector3 neon_pos = pos;
                neon_pos.y = buildings[i].size.y - 5;
                DrawCube(neon_pos, buildings[i].size.x * 0.8f, 2, buildings[i].size.z * 0.8f, SKYBLUE);
            }
        }
        
        // Draw AI characters
        for (int i = 0; i < AI_COUNT; i++) {
            DrawCube(ai_chars[i].pos, 1, 2, 1, ai_chars[i].color);
        }
        
        EndMode3D();
        
        // HUD
        DrawText("ALTITUDE:", 10, 10, 20, GREEN);
        DrawText(TextFormat("%.0f m", camera.position.y), 10, 35, 30, WHITE);
        
        DrawText("FPS:", SCREEN_WIDTH - 100, 10, 20, GREEN);
        DrawText(TextFormat("%d", GetFPS()), SCREEN_WIDTH - 100, 35, 30, WHITE);
        
        // Crosshair
        DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 20, GREEN);
        
        if (camera.position.y < 50) {
            DrawText("LOW ALTITUDE!", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT - 50, 25, RED);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    
    return 0;
}
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define BUILDING_COUNT 200
#define AI_COUNT 500

typedef struct {
    Vector3 pos;
    Vector3 size;
    Color color;
    bool has_neon;
    float neon_phase;
} Building;

typedef struct {
    Vector3 pos;
    Vector3 vel;
    Color color;
    bool fleeing;
} AIChar;

Building buildings[BUILDING_COUNT];
AIChar ai_chars[AI_COUNT];
Camera3D camera = { 0 };
float aircraft_speed = 50.0f;
float yaw = 0.0f;   // Aircraft rotation around Y axis
float pitch = 0.0f; // Aircraft rotation around X axis
Vector3 aircraft_forward = { 0.0f, 0.0f, 1.0f };

void InitWorld() {
    // Initialize camera - cockpit view
    camera.position = (Vector3){ 0.0f, 100.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 100.0f, 10.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Generate cyberpunk city
    for (int i = 0; i < BUILDING_COUNT; i++) {
        buildings[i].pos.x = GetRandomValue(-2000, 2000);
        buildings[i].pos.z = GetRandomValue(-2000, 2000);
        buildings[i].pos.y = 0;
        
        buildings[i].size.x = GetRandomValue(40, 100);
        buildings[i].size.z = GetRandomValue(40, 100);
        buildings[i].size.y = GetRandomValue(100, 600);
        
        // Cyberpunk colors
        int color_type = GetRandomValue(0, 4);
        switch(color_type) {
            case 0: buildings[i].color = (Color){20, 20, 40, 255}; break;
            case 1: buildings[i].color = (Color){30, 20, 50, 255}; break;
            case 2: buildings[i].color = (Color){40, 30, 30, 255}; break;
            case 3: buildings[i].color = (Color){20, 30, 40, 255}; break;
            case 4: buildings[i].color = (Color){30, 30, 50, 255}; break;
        }
        
        buildings[i].has_neon = GetRandomValue(0, 100) < 70;
        buildings[i].neon_phase = GetRandomValue(0, 360);
    }
    
    // Spawn AI characters
    for (int i = 0; i < AI_COUNT; i++) {
        ai_chars[i].pos.x = GetRandomValue(-2000, 2000);
        ai_chars[i].pos.z = GetRandomValue(-2000, 2000);
        ai_chars[i].pos.y = 1.0f;
        
        ai_chars[i].vel.x = GetRandomValue(-10, 10) / 10.0f;
        ai_chars[i].vel.z = GetRandomValue(-10, 10) / 10.0f;
        ai_chars[i].vel.y = 0;
        
        ai_chars[i].color = (Color){
            GetRandomValue(150, 255),
            GetRandomValue(100, 200),
            GetRandomValue(100, 200),
            255
        };
        
        ai_chars[i].fleeing = false;
    }
}

void UpdateWorld() {
    float deltaTime = GetFrameTime();
    
    // Mouse look controls
    Vector2 mouseDelta = GetMouseDelta();
    yaw -= mouseDelta.x * 0.003f;
    pitch -= mouseDelta.y * 0.003f;
    
    // Clamp pitch to prevent over-rotation
    if (pitch > 1.5f) pitch = 1.5f;
    if (pitch < -1.5f) pitch = -1.5f;
    
    // Calculate aircraft forward direction based on yaw and pitch
    aircraft_forward.x = sinf(yaw) * cosf(pitch);
    aircraft_forward.y = sinf(pitch);
    aircraft_forward.z = cosf(yaw) * cosf(pitch);
    aircraft_forward = Vector3Normalize(aircraft_forward);
    
    // Flight controls - WASD now control aircraft movement
    Vector3 right = Vector3CrossProduct(aircraft_forward, (Vector3){0, 1, 0});
    right = Vector3Normalize(right);
    
    // Strafe controls
    if (IsKeyDown(KEY_A)) {
        camera.position = Vector3Add(camera.position, Vector3Scale(right, -aircraft_speed * 0.5f * deltaTime));
    }
    if (IsKeyDown(KEY_D)) {
        camera.position = Vector3Add(camera.position, Vector3Scale(right, aircraft_speed * 0.5f * deltaTime));
    }
    
    // Forward/backward (always moving forward at current speed)
    camera.position = Vector3Add(camera.position, Vector3Scale(aircraft_forward, aircraft_speed * deltaTime));
    
    // Altitude controls
    if (IsKeyDown(KEY_SPACE)) camera.position.y += aircraft_speed * deltaTime;
    if (IsKeyDown(KEY_LEFT_CONTROL)) camera.position.y -= aircraft_speed * deltaTime;
    
    // Pitch controls with W/S
    if (IsKeyDown(KEY_W)) pitch += deltaTime;
    if (IsKeyDown(KEY_S)) pitch -= deltaTime;
    
    // Clamp altitude
    if (camera.position.y < 5.0f) camera.position.y = 5.0f;
    if (camera.position.y > 1000.0f) camera.position.y = 1000.0f;
    
    // Speed control
    if (IsKeyDown(KEY_Q)) aircraft_speed -= 50.0f * deltaTime;
    if (IsKeyDown(KEY_E)) aircraft_speed += 50.0f * deltaTime;
    if (aircraft_speed < 0) aircraft_speed = 0;
    if (aircraft_speed > 300) aircraft_speed = 300;
    
    // Update camera target based on aircraft direction
    camera.target = Vector3Add(camera.position, aircraft_forward);
    
    // Update AI characters
    for (int i = 0; i < AI_COUNT; i++) {
        ai_chars[i].pos.x += ai_chars[i].vel.x;
        ai_chars[i].pos.z += ai_chars[i].vel.z;
        
        // Bounce off world edges
        if (fabs(ai_chars[i].pos.x) > 2500) ai_chars[i].vel.x *= -1;
        if (fabs(ai_chars[i].pos.z) > 2500) ai_chars[i].vel.z *= -1;
        
        // Check if aircraft is low and close
        float dist = Vector3Distance(camera.position, ai_chars[i].pos);
        if (camera.position.y < 50 && dist < 100) {
            ai_chars[i].fleeing = true;
            // Run away from aircraft
            Vector3 away = Vector3Subtract(ai_chars[i].pos, camera.position);
            away = Vector3Normalize(away);
            ai_chars[i].vel.x = away.x * 5.0f;
            ai_chars[i].vel.z = away.z * 5.0f;
        } else if (ai_chars[i].fleeing && dist > 200) {
            ai_chars[i].fleeing = false;
            ai_chars[i].vel.x = GetRandomValue(-10, 10) / 10.0f;
            ai_chars[i].vel.z = GetRandomValue(-10, 10) / 10.0f;
        }
    }
}

void DrawWorld() {
    BeginDrawing();
    
    // Night sky
    ClearBackground((Color){10, 10, 30, 255});
    
    BeginMode3D(camera);
    
    // Draw ground grid
    DrawGrid(100, 50.0f);
    
    // Draw buildings
    for (int i = 0; i < BUILDING_COUNT; i++) {
        Vector3 pos = buildings[i].pos;
        pos.y += buildings[i].size.y / 2;
        
        // Building body
        DrawCube(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, buildings[i].color);
        DrawCubeWires(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, BLACK);
        
        // Neon lights
        if (buildings[i].has_neon) {
            float pulse = sinf((GetTime() + buildings[i].neon_phase) * 3.0f) * 0.5f + 0.5f;
            Color neon = (Color){0, 255 * pulse, 255 * pulse, 255};
            
            Vector3 neon_pos = pos;
            neon_pos.y = buildings[i].size.y - 10;
            DrawCube(neon_pos, buildings[i].size.x * 0.9f, 2, buildings[i].size.z * 0.9f, neon);
        }
        
        // Windows (simple dots)
        int window_rows = buildings[i].size.y / 10;
        int window_cols = buildings[i].size.x / 10;
        
        for (int r = 0; r < window_rows; r += 2) {
            for (int c = 0; c < window_cols; c += 2) {
                if (GetRandomValue(0, 100) < 80) {
                    Vector3 window_pos = {
                        pos.x - buildings[i].size.x/2 + c * 10 + 5,
                        pos.y - buildings[i].size.y/2 + r * 10 + 5,
                        pos.z - buildings[i].size.z/2 - 0.1f
                    };
                    DrawCube(window_pos, 4, 4, 0.2f, YELLOW);
                }
            }
        }
    }
    
    // Draw AI characters
    for (int i = 0; i < AI_COUNT; i++) {
        Color color = ai_chars[i].fleeing ? RED : ai_chars[i].color;
        DrawCapsule(ai_chars[i].pos, 
                   (Vector3){ai_chars[i].pos.x, ai_chars[i].pos.y + 2, ai_chars[i].pos.z},
                   0.5f, 4, 4, color);
        
        // Shadow
        DrawCylinder((Vector3){ai_chars[i].pos.x, 0.1f, ai_chars[i].pos.z}, 
                    1.0f, 1.0f, 0.1f, 8, Fade(BLACK, 0.5f));
    }
    
    EndMode3D();
    
    // Cockpit HUD
    DrawRectangle(0, 0, SCREEN_WIDTH, 100, Fade(BLACK, 0.7f));
    DrawRectangle(0, SCREEN_HEIGHT - 150, SCREEN_WIDTH, 150, Fade(BLACK, 0.7f));
    
    // HUD text
    DrawText("ALTITUDE", 20, 10, 20, SKYBLUE);
    DrawText(TextFormat("%.0f m", camera.position.y), 20, 35, 30, WHITE);
    
    DrawText("SPEED", 200, 10, 20, SKYBLUE);
    DrawText(TextFormat("%.0f m/s", aircraft_speed), 200, 35, 30, WHITE);
    
    // Debug position
    DrawText("POS", 400, 10, 20, SKYBLUE);
    DrawText(TextFormat("X:%.0f Y:%.0f Z:%.0f", camera.position.x, camera.position.y, camera.position.z), 400, 35, 20, WHITE);
    
    // Debug rotation
    DrawText("YAW/PITCH", 700, 10, 20, SKYBLUE);
    DrawText(TextFormat("Y:%.2f P:%.2f", yaw, pitch), 700, 35, 20, WHITE);
    
    // Crosshair
    DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 40, SKYBLUE);
    DrawLine(SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 - 20, SCREEN_HEIGHT/2, SKYBLUE);
    DrawLine(SCREEN_WIDTH/2 + 20, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 + 50, SCREEN_HEIGHT/2, SKYBLUE);
    DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 50, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 20, SKYBLUE);
    DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 20, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 50, SKYBLUE);
    
    // Instructions
    DrawText("Mouse: Look | W/S: Pitch | A/D: Strafe | Q/E: Speed | Space/Ctrl: Altitude | ESC: Exit", 
             10, SCREEN_HEIGHT - 30, 20, WHITE);
    
    // Low altitude warning
    if (camera.position.y < 50) {
        DrawText("! LOW ALTITUDE !", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT - 100, 30, RED);
    }
    
    DrawFPS(SCREEN_WIDTH - 100, 10);
    
    EndDrawing();
}

int main() {
    // Initialize window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat - 3D Cyberpunk City Flight");
    SetTargetFPS(60);
    DisableCursor();
    
    // Ensure window is ready
    if (!IsWindowReady()) {
        printf("ERROR: Window failed to initialize!\n");
        return 1;
    }
    
    // Initialize world
    InitWorld();
    
    printf("=== SKY COMBAT 3D WORLD ===\n");
    printf("Flying through cyberpunk city with %d buildings and %d AI characters\n", 
           BUILDING_COUNT, AI_COUNT);
    printf("Controls:\n");
    printf("  Mouse - Look around\n");
    printf("  WASD - Move camera\n");
    printf("  Q/E - Adjust speed\n");
    printf("  Space/Ctrl - Up/Down\n");
    printf("  ESC - Exit\n");
    
    // Main game loop
    while (!WindowShouldClose()) {
        UpdateWorld();
        DrawWorld();
    }
    
    CloseWindow();
    
    return 0;
}
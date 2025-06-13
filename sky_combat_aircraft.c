/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BUILDING_COUNT 150
#define AI_COUNT 300

typedef struct {
    Vector3 position;
    Vector3 velocity;
    float pitch;    // Up/down rotation
    float yaw;      // Left/right rotation
    float roll;     // Banking
    float throttle; // Speed control
} Aircraft;

typedef struct {
    Vector3 pos;
    Vector3 size;
    Color color;
    bool has_neon;
    float neon_pulse;
} Building;

typedef struct {
    Vector3 pos;
    Color color;
    bool fleeing;
    Vector3 flee_vel;
} AIChar;

Building buildings[BUILDING_COUNT];
AIChar ai_chars[AI_COUNT];
Aircraft player = { 0 };
Camera3D cockpit_camera = { 0 };

void InitAircraft() {
    player.position = (Vector3){ 0, 200, 0 };
    player.velocity = (Vector3){ 0, 0, 50 };  // Start moving forward
    player.pitch = 0;
    player.yaw = 0;
    player.roll = 0;
    player.throttle = 50;
}

void UpdateAircraft(float dt) {
    // Pitch controls (up/down)
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        player.pitch -= 60 * dt;  // Pitch up
        if (player.pitch < -80) player.pitch = -80;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        player.pitch += 60 * dt;  // Pitch down
        if (player.pitch > 80) player.pitch = 80;
    }
    
    // Yaw/Roll controls (turning)
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player.yaw -= 90 * dt;    // Turn left
        player.roll = Lerp(player.roll, -30, 5 * dt);  // Bank left
    }
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player.yaw += 90 * dt;    // Turn right
        player.roll = Lerp(player.roll, 30, 5 * dt);   // Bank right
    }
    else {
        player.roll = Lerp(player.roll, 0, 3 * dt);    // Level out
    }
    
    // Throttle controls
    if (IsKeyDown(KEY_Q)) {
        player.throttle -= 100 * dt;
        if (player.throttle < 0) player.throttle = 0;
    }
    if (IsKeyDown(KEY_E)) {
        player.throttle += 100 * dt;
        if (player.throttle > 200) player.throttle = 200;
    }
    
    // Boost
    float boost = IsKeyDown(KEY_LEFT_SHIFT) ? 2.0f : 1.0f;
    float effective_speed = player.throttle * boost;
    
    // Calculate velocity based on aircraft orientation
    float yaw_rad = player.yaw * DEG2RAD;
    float pitch_rad = player.pitch * DEG2RAD;
    
    player.velocity.x = effective_speed * cosf(yaw_rad) * cosf(pitch_rad);
    player.velocity.y = effective_speed * sinf(pitch_rad);
    player.velocity.z = effective_speed * sinf(yaw_rad) * cosf(pitch_rad);
    
    // Update position
    player.position.x += player.velocity.x * dt;
    player.position.y += player.velocity.y * dt;
    player.position.z += player.velocity.z * dt;
    
    // Keep above ground
    if (player.position.y < 5) {
        player.position.y = 5;
        player.velocity.y = 0;
    }
    
    // Update camera to cockpit view
    cockpit_camera.position = player.position;
    
    // Look direction based on aircraft orientation
    float look_distance = 100.0f;
    cockpit_camera.target.x = player.position.x + look_distance * cosf(yaw_rad) * cosf(pitch_rad);
    cockpit_camera.target.y = player.position.y + look_distance * sinf(pitch_rad);
    cockpit_camera.target.z = player.position.z + look_distance * sinf(yaw_rad) * cosf(pitch_rad);
    
    // Roll the camera
    float roll_rad = player.roll * DEG2RAD;
    cockpit_camera.up.x = sinf(roll_rad);
    cockpit_camera.up.y = cosf(roll_rad);
    cockpit_camera.up.z = 0;
}

void DrawCockpitHUD() {
    // Artificial horizon
    int horizon_y = SCREEN_HEIGHT/2 - (int)(player.pitch * 2);
    DrawLine(0, horizon_y, SCREEN_WIDTH, horizon_y, Fade(SKYBLUE, 0.5f));
    
    // Altitude
    DrawText("ALT", 20, 20, 20, GREEN);
    DrawText(TextFormat("%.0f m", player.position.y), 20, 45, 30, WHITE);
    
    // Speed
    DrawText("SPD", 20, 90, 20, GREEN);
    DrawText(TextFormat("%.0f m/s", player.throttle), 20, 115, 30, WHITE);
    
    // Throttle bar
    DrawRectangle(20, 160, 20, 200, Fade(GREEN, 0.3f));
    DrawRectangle(20, 360 - (int)(player.throttle), 20, (int)(player.throttle), GREEN);
    DrawText("THR", 15, 370, 16, GREEN);
    
    // Heading
    DrawText("HDG", SCREEN_WIDTH/2 - 30, 20, 20, GREEN);
    DrawText(TextFormat("%03d°", ((int)player.yaw + 360) % 360), SCREEN_WIDTH/2 - 40, 45, 30, WHITE);
    
    // Crosshair
    DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, GREEN);
    DrawLine(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 - 20, SCREEN_HEIGHT/2, GREEN);
    DrawLine(SCREEN_WIDTH/2 + 20, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 + 40, SCREEN_HEIGHT/2, GREEN);
    DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 40, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 20, GREEN);
    DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 20, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 40, GREEN);
    
    // Low altitude warning
    if (player.position.y < 50) {
        DrawText("! PULL UP !", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT - 100, 30, RED);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(RED, 0.1f));
    }
    
    // Instructions
    DrawText("AIRCRAFT CONTROLS: W/S=Pitch  A/D=Turn  Q/E=Throttle  Shift=Boost", 
             10, SCREEN_HEIGHT - 25, 16, Fade(WHITE, 0.7f));
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat 3D - Aircraft Flight");
    SetTargetFPS(60);
    
    InitAircraft();
    
    // Setup cockpit camera
    cockpit_camera.position = player.position;
    cockpit_camera.target = (Vector3){ 0, 200, 100 };
    cockpit_camera.up = (Vector3){ 0, 1, 0 };
    cockpit_camera.fovy = 90.0f;
    cockpit_camera.projection = CAMERA_PERSPECTIVE;
    
    // Generate cyberpunk city
    for (int i = 0; i < BUILDING_COUNT; i++) {
        buildings[i].pos.x = (float)(GetRandomValue(-20, 20) * 100);
        buildings[i].pos.z = (float)(GetRandomValue(-20, 20) * 100);
        buildings[i].pos.y = 0;
        
        buildings[i].size.x = GetRandomValue(40, 100);
        buildings[i].size.z = GetRandomValue(40, 100);
        buildings[i].size.y = GetRandomValue(50, 500);
        
        buildings[i].color = (Color){
            GetRandomValue(10, 40),
            GetRandomValue(10, 40),
            GetRandomValue(20, 60),
            255
        };
        
        buildings[i].has_neon = GetRandomValue(0, 100) < 70;
        buildings[i].neon_pulse = GetRandomValue(0, 628) / 100.0f;
    }
    
    // Spawn AI
    for (int i = 0; i < AI_COUNT; i++) {
        ai_chars[i].pos.x = (float)GetRandomValue(-2000, 2000);
        ai_chars[i].pos.z = (float)GetRandomValue(-2000, 2000);
        ai_chars[i].pos.y = 1.0f;
        ai_chars[i].color = (Color){255, 150, 0, 255};
        ai_chars[i].fleeing = false;
        ai_chars[i].flee_vel = (Vector3){0, 0, 0};
    }
    
    printf("\n=== SKY COMBAT 3D - AIRCRAFT FLIGHT ===\n");
    printf("You are flying an aircraft through a cyberpunk city!\n\n");
    printf("AIRCRAFT CONTROLS:\n");
    printf("  W/S or Up/Down  - Pitch (climb/dive)\n");
    printf("  A/D or Left/Right - Turn (with banking)\n");
    printf("  Q/E - Throttle control\n");
    printf("  Shift - Afterburner boost\n");
    printf("  ESC - Exit\n\n");
    printf("Fly low to scatter the crowds!\n");
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // Update aircraft physics
        UpdateAircraft(dt);
        
        // Update AI
        for (int i = 0; i < AI_COUNT; i++) {
            float dist = Vector3Distance(player.position, ai_chars[i].pos);
            
            if (player.position.y < 50 && dist < 100) {
                if (!ai_chars[i].fleeing) {
                    ai_chars[i].fleeing = true;
                    // Calculate flee direction
                    Vector3 flee_dir = Vector3Subtract(ai_chars[i].pos, player.position);
                    flee_dir.y = 0;
                    flee_dir = Vector3Normalize(flee_dir);
                    ai_chars[i].flee_vel = Vector3Scale(flee_dir, 5.0f);
                }
                ai_chars[i].color = RED;
            } else {
                ai_chars[i].fleeing = false;
                ai_chars[i].color = (Color){255, 150, 0, 255};
            }
            
            // Update fleeing AI position
            if (ai_chars[i].fleeing) {
                ai_chars[i].pos = Vector3Add(ai_chars[i].pos, Vector3Scale(ai_chars[i].flee_vel, dt));
            }
        }
        
        // Draw
        BeginDrawing();
        ClearBackground((Color){5, 5, 20, 255}); // Night sky
        
        BeginMode3D(cockpit_camera);
        
        // Ground
        DrawPlane((Vector3){0, 0, 0}, (Vector2){4000, 4000}, (Color){20, 20, 30, 255});
        
        // Grid lines
        for (int i = -20; i <= 20; i++) {
            DrawLine3D((Vector3){i*100, 0, -2000}, (Vector3){i*100, 0, 2000}, Fade(BLUE, 0.3f));
            DrawLine3D((Vector3){-2000, 0, i*100}, (Vector3){2000, 0, i*100}, Fade(BLUE, 0.3f));
        }
        
        // Buildings
        for (int i = 0; i < BUILDING_COUNT; i++) {
            Vector3 pos = buildings[i].pos;
            pos.y = buildings[i].size.y / 2;
            
            DrawCube(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, buildings[i].color);
            DrawCubeWires(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, BLACK);
            
            // Neon
            if (buildings[i].has_neon) {
                float pulse = sinf(GetTime() * 2 + buildings[i].neon_pulse) * 0.5f + 0.5f;
                Color neon = (Color){0, (int)(200 * pulse), (int)(255 * pulse), 255};
                Vector3 neon_pos = pos;
                neon_pos.y = buildings[i].size.y - 5;
                DrawCube(neon_pos, buildings[i].size.x * 0.8f, 3, buildings[i].size.z * 0.8f, neon);
            }
        }
        
        // AI characters
        for (int i = 0; i < AI_COUNT; i++) {
            DrawCube(ai_chars[i].pos, 1, 2, 1, ai_chars[i].color);
            if (ai_chars[i].fleeing) {
                // Motion lines
                DrawLine3D(ai_chars[i].pos, 
                          Vector3Subtract(ai_chars[i].pos, Vector3Scale(ai_chars[i].flee_vel, 0.5f)), 
                          RED);
            }
        }
        
        EndMode3D();
        
        // Draw HUD
        DrawCockpitHUD();
        
        DrawFPS(SCREEN_WIDTH - 100, 10);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
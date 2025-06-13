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
    float pitch;
    float yaw;
    float roll;
    float throttle;
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
int gamepad = 0;  // First gamepad

void InitAircraft() {
    player.position = (Vector3){ 0, 200, 0 };
    player.velocity = (Vector3){ 0, 0, 50 };
    player.pitch = 0;
    player.yaw = 0;
    player.roll = 0;
    player.throttle = 50;
}

void UpdateAircraftWithJoystick(float dt) {
    // Check for gamepad
    if (IsGamepadAvailable(gamepad)) {
        // Right stick for pitch and roll (like a real flight stick)
        float stick_x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X);
        float stick_y = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y);
        
        // Pitch (pull back to climb, push forward to dive)
        player.pitch -= stick_y * 60 * dt;
        if (player.pitch < -80) player.pitch = -80;
        if (player.pitch > 80) player.pitch = 80;
        
        // Roll and yaw (left/right on stick)
        player.yaw += stick_x * 90 * dt;
        player.roll = Lerp(player.roll, -stick_x * 30, 5 * dt);
        
        // Left stick for fine control (optional)
        float left_x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
        player.yaw += left_x * 45 * dt;  // Additional yaw control
        
        // Triggers for throttle
        float right_trigger = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_TRIGGER);
        float left_trigger = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_TRIGGER);
        
        // Right trigger increases throttle
        if (right_trigger > 0.1f) {
            player.throttle += right_trigger * 100 * dt;
        }
        // Left trigger decreases throttle
        if (left_trigger > 0.1f) {
            player.throttle -= left_trigger * 100 * dt;
        }
        
        // Buttons
        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {  // A/Cross
            player.throttle += 100 * dt;  // Boost
        }
        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) { // B/Circle
            player.throttle -= 100 * dt;  // Brake
        }
        
        // D-pad for trim adjustments
        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
            player.pitch -= 30 * dt;
        }
        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
            player.pitch += 30 * dt;
        }
    }
    
    // Keyboard fallback
    if (IsKeyDown(KEY_W)) player.pitch -= 60 * dt;
    if (IsKeyDown(KEY_S)) player.pitch += 60 * dt;
    if (IsKeyDown(KEY_A)) {
        player.yaw -= 90 * dt;
        player.roll = Lerp(player.roll, -30, 5 * dt);
    } else if (IsKeyDown(KEY_D)) {
        player.yaw += 90 * dt;
        player.roll = Lerp(player.roll, 30, 5 * dt);
    } else if (!IsGamepadAvailable(gamepad)) {
        player.roll = Lerp(player.roll, 0, 3 * dt);
    }
    
    if (IsKeyDown(KEY_Q)) player.throttle -= 100 * dt;
    if (IsKeyDown(KEY_E)) player.throttle += 100 * dt;
    
    // Clamp throttle
    if (player.throttle < 0) player.throttle = 0;
    if (player.throttle > 200) player.throttle = 200;
    
    // Calculate velocity
    float yaw_rad = player.yaw * DEG2RAD;
    float pitch_rad = player.pitch * DEG2RAD;
    
    player.velocity.x = player.throttle * cosf(yaw_rad) * cosf(pitch_rad);
    player.velocity.y = player.throttle * sinf(pitch_rad);
    player.velocity.z = player.throttle * sinf(yaw_rad) * cosf(pitch_rad);
    
    // Update position
    player.position.x += player.velocity.x * dt;
    player.position.y += player.velocity.y * dt;
    player.position.z += player.velocity.z * dt;
    
    if (player.position.y < 5) {
        player.position.y = 5;
        player.velocity.y = 0;
    }
    
    // Update camera
    cockpit_camera.position = player.position;
    
    float look_distance = 100.0f;
    cockpit_camera.target.x = player.position.x + look_distance * cosf(yaw_rad) * cosf(pitch_rad);
    cockpit_camera.target.y = player.position.y + look_distance * sinf(pitch_rad);
    cockpit_camera.target.z = player.position.z + look_distance * sinf(yaw_rad) * cosf(pitch_rad);
    
    float roll_rad = player.roll * DEG2RAD;
    cockpit_camera.up.x = sinf(roll_rad);
    cockpit_camera.up.y = cosf(roll_rad);
    cockpit_camera.up.z = 0;
}

void DrawJoystickStatus() {
    int y_pos = 100;
    
    if (IsGamepadAvailable(gamepad)) {
        const char* name = GetGamepadName(gamepad);
        DrawText("JOYSTICK CONNECTED", SCREEN_WIDTH - 250, y_pos, 16, GREEN);
        DrawText(name, SCREEN_WIDTH - 250, y_pos + 20, 12, GREEN);
        
        // Show stick positions
        float rx = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X);
        float ry = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y);
        
        DrawText("RIGHT STICK:", SCREEN_WIDTH - 250, y_pos + 50, 14, WHITE);
        DrawCircle(SCREEN_WIDTH - 170, y_pos + 90, 30, Fade(WHITE, 0.3f));
        DrawCircle(SCREEN_WIDTH - 170 + (int)(rx * 25), 
                  y_pos + 90 + (int)(ry * 25), 5, GREEN);
        
        // Throttle from triggers
        float throttle_input = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_TRIGGER) - 
                              GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_TRIGGER);
        DrawText("TRIGGERS:", SCREEN_WIDTH - 250, y_pos + 130, 14, WHITE);
        DrawRectangle(SCREEN_WIDTH - 250, y_pos + 150, 100, 10, Fade(WHITE, 0.3f));
        DrawRectangle(SCREEN_WIDTH - 250, y_pos + 150, 
                     50 + (int)(throttle_input * 50), 10, GREEN);
    } else {
        DrawText("NO JOYSTICK", SCREEN_WIDTH - 200, y_pos, 16, RED);
        DrawText("Using keyboard", SCREEN_WIDTH - 200, y_pos + 20, 14, WHITE);
    }
}

void DrawCockpitHUD() {
    // Altitude
    DrawText("ALT", 20, 20, 20, GREEN);
    DrawText(TextFormat("%.0f m", player.position.y), 20, 45, 30, WHITE);
    
    // Speed
    DrawText("SPD", 20, 90, 20, GREEN);
    DrawText(TextFormat("%.0f m/s", player.throttle), 20, 115, 30, WHITE);
    
    // Throttle
    DrawRectangle(20, 160, 20, 200, Fade(GREEN, 0.3f));
    DrawRectangle(20, 360 - (int)(player.throttle), 20, (int)(player.throttle), GREEN);
    DrawText("THR", 15, 370, 16, GREEN);
    
    // Heading
    DrawText("HDG", SCREEN_WIDTH/2 - 30, 20, 20, GREEN);
    DrawText(TextFormat("%03d°", ((int)player.yaw + 360) % 360), SCREEN_WIDTH/2 - 40, 45, 30, WHITE);
    
    // Pitch indicator
    int horizon_y = SCREEN_HEIGHT/2 - (int)(player.pitch * 2);
    DrawLine(100, horizon_y, SCREEN_WIDTH - 100, horizon_y, Fade(SKYBLUE, 0.5f));
    
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
    
    // Joystick status
    DrawJoystickStatus();
    
    // Controls
    if (IsGamepadAvailable(gamepad)) {
        DrawText("JOYSTICK: Right Stick=Fly  Triggers=Throttle  A=Boost  B=Brake", 
                 10, SCREEN_HEIGHT - 25, 14, Fade(WHITE, 0.7f));
    } else {
        DrawText("KEYBOARD: W/S=Pitch  A/D=Turn  Q/E=Throttle  (Connect joystick for better control!)", 
                 10, SCREEN_HEIGHT - 25, 14, Fade(WHITE, 0.7f));
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat 3D - Joystick Flight");
    SetTargetFPS(60);
    
    InitAircraft();
    
    cockpit_camera.position = player.position;
    cockpit_camera.target = (Vector3){ 0, 200, 100 };
    cockpit_camera.up = (Vector3){ 0, 1, 0 };
    cockpit_camera.fovy = 90.0f;
    cockpit_camera.projection = CAMERA_PERSPECTIVE;
    
    // Generate city
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
    }
    
    printf("\n=== SKY COMBAT 3D - JOYSTICK SUPPORT ===\n");
    
    // Check for joysticks
    printf("\nChecking for joysticks...\n");
    for (int i = 0; i < 4; i++) {
        if (IsGamepadAvailable(i)) {
            printf("  Joystick %d: %s\n", i, GetGamepadName(i));
            gamepad = i;
            break;
        }
    }
    
    if (!IsGamepadAvailable(gamepad)) {
        printf("  No joystick detected - using keyboard\n");
    }
    
    printf("\nJOYSTICK CONTROLS:\n");
    printf("  Right Stick - Fly (pull back to climb)\n");
    printf("  Left Stick - Additional yaw\n");
    printf("  Right Trigger - Increase throttle\n");
    printf("  Left Trigger - Decrease throttle\n");
    printf("  A/Cross - Boost\n");
    printf("  B/Circle - Brake\n");
    printf("  D-Pad - Trim adjustments\n");
    
    printf("\nKEYBOARD FALLBACK:\n");
    printf("  W/S - Pitch\n");
    printf("  A/D - Turn\n");
    printf("  Q/E - Throttle\n");
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        UpdateAircraftWithJoystick(dt);
        
        // Update AI
        for (int i = 0; i < AI_COUNT; i++) {
            float dist = Vector3Distance(player.position, ai_chars[i].pos);
            
            if (player.position.y < 50 && dist < 100) {
                if (!ai_chars[i].fleeing) {
                    ai_chars[i].fleeing = true;
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
            
            if (ai_chars[i].fleeing) {
                ai_chars[i].pos = Vector3Add(ai_chars[i].pos, Vector3Scale(ai_chars[i].flee_vel, dt));
            }
        }
        
        BeginDrawing();
        ClearBackground((Color){5, 5, 20, 255});
        
        BeginMode3D(cockpit_camera);
        
        DrawPlane((Vector3){0, 0, 0}, (Vector2){4000, 4000}, (Color){20, 20, 30, 255});
        
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
            
            if (buildings[i].has_neon) {
                float pulse = sinf(GetTime() * 2 + buildings[i].neon_pulse) * 0.5f + 0.5f;
                Color neon = (Color){0, (int)(200 * pulse), (int)(255 * pulse), 255};
                Vector3 neon_pos = pos;
                neon_pos.y = buildings[i].size.y - 5;
                DrawCube(neon_pos, buildings[i].size.x * 0.8f, 3, buildings[i].size.z * 0.8f, neon);
            }
        }
        
        // AI
        for (int i = 0; i < AI_COUNT; i++) {
            DrawCube(ai_chars[i].pos, 1, 2, 1, ai_chars[i].color);
            if (ai_chars[i].fleeing) {
                DrawLine3D(ai_chars[i].pos, 
                          Vector3Subtract(ai_chars[i].pos, Vector3Scale(ai_chars[i].flee_vel, 0.5f)), 
                          RED);
            }
        }
        
        EndMode3D();
        
        DrawCockpitHUD();
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
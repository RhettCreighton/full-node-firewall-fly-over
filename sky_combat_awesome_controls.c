/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#define _XOPEN_SOURCE 500

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BUILDING_COUNT 150
#define AI_COUNT 300
#define AIRCRAFT_MIN_SPEED 30.0f   // Keep it fast!
#define AIRCRAFT_MAX_SPEED 300.0f  // Super speed for cool flying!

// Joystick state
typedef struct {
    int fd;
    char name[256];
    int num_axes;
    int num_buttons;
    int16_t axes[32];
    uint8_t buttons[32];
    int connected;
} JoystickState;

// Aircraft - using the awesome original physics!
typedef struct {
    Vector3 position;
    float yaw;
    float pitch;
    float roll;
    float speed;
    float altitude;
} Aircraft;

// Buildings
typedef struct {
    Vector3 pos;
    Vector3 size;
    Color color;
    bool has_neon;
    float neon_pulse;
} Building;

// AI
typedef struct {
    Vector3 pos;
    Color color;
    bool fleeing;
    Vector3 flee_vel;
} AIChar;

// Global state
JoystickState joystick = {0};
Aircraft player = {{0, 50, 0}, 0, 0, 0, 120, 50};  // Enhanced speed!
Building buildings[BUILDING_COUNT];
AIChar ai_chars[AI_COUNT];
Camera3D camera = {0};
pthread_t joystick_thread;
int joystick_thread_running = 0;

// Open joystick
int open_joystick() {
    for (int i = 0; i < 4; i++) {
        char device[32];
        sprintf(device, "/dev/input/js%d", i);
        
        joystick.fd = open(device, O_RDONLY | O_NONBLOCK);
        if (joystick.fd >= 0) {
            ioctl(joystick.fd, JSIOCGNAME(sizeof(joystick.name)), joystick.name);
            ioctl(joystick.fd, JSIOCGAXES, &joystick.num_axes);
            ioctl(joystick.fd, JSIOCGBUTTONS, &joystick.num_buttons);
            
            printf("✓ Joystick connected: %s\n", joystick.name);
            printf("  Axes: %d, Buttons: %d\n", joystick.num_axes, joystick.num_buttons);
            printf("  IMPORTANT: Using axis 5 for right stick Y (per specification)\n");
            printf("  Right Stick: X=axis2, Y=axis5\n");
            printf("  Left Stick:  X=axis0, Y=axis1\n");
            
            joystick.connected = 1;
            return 1;
        }
    }
    return 0;
}

// Joystick reader thread
void* joystick_reader(void* arg) {
    struct js_event event;
    
    while (joystick_thread_running && joystick.connected) {
        int bytes = read(joystick.fd, &event, sizeof(event));
        
        if (bytes == sizeof(event)) {
            switch (event.type & ~JS_EVENT_INIT) {
                case JS_EVENT_AXIS:
                    if (event.number < 32) {
                        joystick.axes[event.number] = event.value;
                    }
                    break;
                    
                case JS_EVENT_BUTTON:
                    if (event.number < 32) {
                        joystick.buttons[event.number] = event.value;
                    }
                    break;
            }
        }
        
        usleep(1000);
    }
    
    return NULL;
}

// Normalize axis with deadzone
float normalize_axis(int16_t value) {
    if (abs(value) < 3000) return 0;  // Deadzone
    float normalized = value / 32768.0f;
    // Clamp to ensure we don't exceed -1 to 1 range
    return Clamp(normalized, -1.0f, 1.0f);
}

// THE AWESOME UPDATE FUNCTION FROM THE ORIGINAL!
void update_aircraft_awesome(float dt) {
    float inputX = 0;
    float inputY = 0;
    
    if (joystick.connected) {
        // SPECIFICATION: Right stick Y MUST use axis 5 (not axis 3)
        // Right stick X uses axis 2, Right stick Y uses axis 5
        inputX = -normalize_axis(joystick.axes[2]);  // Right stick X
        inputY = -normalize_axis(joystick.axes[5]);  // Right stick Y (SPEC requirement)
        
        // If no input on right stick, try left stick (axes 0,1)
        if (fabs(inputX) < 0.1f && fabs(inputY) < 0.1f) {
            inputX = -normalize_axis(joystick.axes[0]);  // Left stick X
            inputY = -normalize_axis(joystick.axes[1]);  // Left stick Y
        }
    }
    
    // Keyboard fallback
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) inputY = -1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) inputY = 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) inputX = -1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) inputX = 1;
    
    // THE AWESOME CONTROLS FROM GIT!
    // Pitch and roll with smooth lerping
    player.pitch = Lerp(player.pitch, inputY * 40.0f, 8.0f * dt);
    player.roll = Lerp(player.roll, -inputX * 35.0f, 5.0f * dt);
    
    // Yaw based on roll (banking turns) - ENHANCED AGILITY!
    player.yaw += (player.roll / 35.0f) * 150.0f * dt;  // 3x turn rate!
    
    // Speed control based on pitch!
    if (player.pitch < 0) {
        // Climbing slows you down
        player.speed -= (-player.pitch / 40.0f) * 30.0f * dt;
    } else {
        // Diving speeds you up
        player.speed += (player.pitch / 40.0f) * 50.0f * dt;
    }
    
    // Extra controls
    if ((joystick.connected && joystick.buttons[0]) || IsKeyDown(KEY_SPACE)) {
        player.speed += 50.0f * dt;  // Boost
    }
    if ((joystick.connected && joystick.buttons[1]) || IsKeyDown(KEY_LEFT_CONTROL)) {
        player.speed -= 60.0f * dt;  // Brake
    }
    
    player.speed = Clamp(player.speed, AIRCRAFT_MIN_SPEED, AIRCRAFT_MAX_SPEED);
    
    // Update position using aircraft physics
    float yaw_rad = player.yaw * DEG2RAD;
    float pitch_rad = player.pitch * DEG2RAD;
    Vector3 forward = {
        sinf(yaw_rad) * cosf(pitch_rad),
        -sinf(pitch_rad),
        cosf(yaw_rad) * cosf(pitch_rad)
    };
    
    player.position = Vector3Add(player.position, Vector3Scale(forward, player.speed * dt));
    player.altitude = player.position.y;
    
    // Keep above ground
    if (player.position.y < 5) {
        player.position.y = 5;
        player.pitch = 0;  // Level out when hitting ground
    }
    
    // Update camera to follow aircraft
    Vector3 camera_offset = {
        -sinf(yaw_rad) * 50,
        20,
        -cosf(yaw_rad) * 50
    };
    
    camera.position = Vector3Add(player.position, camera_offset);
    camera.position.y += 10;
    
    // Look ahead of aircraft
    camera.target = Vector3Add(player.position, Vector3Scale(forward, 30));
    
    // Bank the camera with roll
    float roll_rad = player.roll * DEG2RAD;
    camera.up = (Vector3){sinf(roll_rad) * 0.5f, cosf(roll_rad), 0};
}

int main() {
    printf("=== SKY COMBAT - AWESOME CONTROLS RESTORED ===\n\n");
    
    // Open joystick
    if (open_joystick()) {
        joystick_thread_running = 1;
        pthread_create(&joystick_thread, NULL, joystick_reader, NULL);
    }
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat 3D - Awesome Controls");
    SetTargetFPS(60);
    
    // Setup camera
    camera.position = (Vector3){0, 60, -50};
    camera.target = player.position;
    camera.up = (Vector3){0, 1, 0};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
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
    
    // AI
    for (int i = 0; i < AI_COUNT; i++) {
        ai_chars[i].pos.x = (float)GetRandomValue(-2000, 2000);
        ai_chars[i].pos.z = (float)GetRandomValue(-2000, 2000);
        ai_chars[i].pos.y = 1.0f;
        ai_chars[i].color = (Color){255, 150, 0, 255};
        ai_chars[i].fleeing = false;
    }
    
    printf("AWESOME CONTROLS RESTORED:\n");
    printf("✓ Banking turns (roll controls yaw)\n");
    printf("✓ Pitch affects speed (dive to go faster)\n");
    printf("✓ Smooth, responsive flight\n");
    printf("✓ Camera follows behind aircraft\n\n");
    
    printf("CONTROLS:\n");
    if (joystick.connected) {
        printf("JOYSTICK: Stick to fly, Button 1 = Boost, Button 2 = Brake\n");
    }
    printf("KEYBOARD: WASD/Arrows to fly, Space = Boost, Ctrl = Brake\n");
    
    printf("Entering main game loop...\n");
    fflush(stdout);
    
    int frame_count = 0;
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        frame_count++;
        
        // Debug output every 60 frames (once per second)
        if (frame_count % 60 == 0 && joystick.connected) {
            printf("Frame %d - FPS: %d | Raw axes: [0]=%d [1]=%d [2]=%d [5]=%d\n", 
                   frame_count, GetFPS(), 
                   joystick.axes[0], joystick.axes[1], 
                   joystick.axes[2], joystick.axes[5]);
            fflush(stdout);
        }
        
        // Update with awesome controls!
        update_aircraft_awesome(dt);
        
        // Update AI
        for (int i = 0; i < AI_COUNT; i++) {
            float dist = Vector3Distance(player.position, ai_chars[i].pos);
            
            if (player.altitude < 50 && dist < 100) {
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
        
        BeginMode3D(camera);
        
        DrawPlane((Vector3){0, 0, 0}, (Vector2){4000, 4000}, (Color){20, 20, 30, 255});
        
        // Grid
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
        
        // Draw aircraft
        DrawCube(player.position, 3, 1, 5, WHITE);
        
        // AI
        for (int i = 0; i < AI_COUNT; i++) {
            DrawCube(ai_chars[i].pos, 1, 2, 1, ai_chars[i].color);
        }
        
        EndMode3D();
        
        // HUD
        DrawText("ALTITUDE", 20, 20, 16, GREEN);
        DrawText(TextFormat("%.0f m", player.altitude), 20, 40, 24, WHITE);
        
        DrawText("SPEED", 20, 80, 16, GREEN);
        int speed_percent = (int)((player.speed - AIRCRAFT_MIN_SPEED) / (AIRCRAFT_MAX_SPEED - AIRCRAFT_MIN_SPEED) * 100);
        DrawText(TextFormat("%d%%", speed_percent), 20, 100, 24, WHITE);
        
        // Speed bar
        DrawRectangle(20, 130, 200, 20, Fade(GREEN, 0.3f));
        DrawRectangle(20, 130, speed_percent * 2, 20, GREEN);
        
        // Pitch indicator
        DrawText("PITCH", 20, 170, 16, GREEN);
        DrawRectangle(20, 190, 100, 10, DARKGRAY);
        DrawRectangle(70 + (int)(player.pitch), 190, 5, 10, YELLOW);
        
        // Roll indicator
        DrawText("ROLL", 20, 210, 16, GREEN);
        DrawRectangle(20, 230, 100, 10, DARKGRAY);
        DrawRectangle(70 + (int)(player.roll * 0.7f), 230, 5, 10, YELLOW);
        
        // Controls reminder
        DrawText("Banking turns • Pitch affects speed • Smooth flight", 
                 SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT - 30, 16, Fade(WHITE, 0.7f));
        
        // Joystick status
        if (joystick.connected) {
            DrawText("JOYSTICK ✓", SCREEN_WIDTH - 120, 20, 16, GREEN);
            
            // Show RIGHT stick position (axes 2,5 per specification)
            float rx = normalize_axis(joystick.axes[2]);
            float ry = normalize_axis(joystick.axes[5]);
            DrawText("RIGHT STICK", SCREEN_WIDTH - 140, 45, 12, WHITE);
            DrawCircle(SCREEN_WIDTH - 60, 70, 20, Fade(WHITE, 0.3f));
            DrawCircle(SCREEN_WIDTH - 60 + (int)(rx * 15), 
                      70 + (int)(ry * 15), 5, GREEN);
            
            // Show LEFT stick position (axes 0,1)
            float lx = normalize_axis(joystick.axes[0]);
            float ly = normalize_axis(joystick.axes[1]);
            DrawText("LEFT STICK", SCREEN_WIDTH - 140, 105, 12, WHITE);
            DrawCircle(SCREEN_WIDTH - 60, 130, 20, Fade(WHITE, 0.3f));
            DrawCircle(SCREEN_WIDTH - 60 + (int)(lx * 15), 
                      130 + (int)(ly * 15), 5, YELLOW);
            
            // Debug axis values
            DrawText(TextFormat("Axis 2 (RX): %d", joystick.axes[2]), SCREEN_WIDTH - 200, 160, 10, GRAY);
            DrawText(TextFormat("Axis 5 (RY): %d", joystick.axes[5]), SCREEN_WIDTH - 200, 175, 10, GRAY);
        }
        
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    // Cleanup
    joystick_thread_running = 0;
    if (joystick.connected) {
        pthread_join(joystick_thread, NULL);
        close(joystick.fd);
    }
    
    CloseWindow();
    return 0;
}
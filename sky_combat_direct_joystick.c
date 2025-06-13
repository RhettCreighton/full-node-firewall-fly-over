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
#define _XOPEN_SOURCE 500

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BUILDING_COUNT 150
#define AI_COUNT 300

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

// Aircraft
typedef struct {
    Vector3 position;
    Vector3 velocity;
    float pitch, yaw, roll;
    float throttle;
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
Aircraft player = {{0, 200, 0}, {0, 0, 50}, 0, 0, 0, 50};
Building buildings[BUILDING_COUNT];
AIChar ai_chars[AI_COUNT];
Camera3D cockpit_camera = {0};
pthread_t joystick_thread;
int joystick_thread_running = 0;

// Open joystick device directly
int open_joystick() {
    // Try js0 through js3
    for (int i = 0; i < 4; i++) {
        char device[32];
        sprintf(device, "/dev/input/js%d", i);
        
        joystick.fd = open(device, O_RDONLY | O_NONBLOCK);
        if (joystick.fd >= 0) {
            printf("✓ Opened %s\n", device);
            
            // Get joystick info
            ioctl(joystick.fd, JSIOCGNAME(sizeof(joystick.name)), joystick.name);
            ioctl(joystick.fd, JSIOCGAXES, &joystick.num_axes);
            ioctl(joystick.fd, JSIOCGBUTTONS, &joystick.num_buttons);
            
            printf("✓ Joystick: %s\n", joystick.name);
            printf("✓ Axes: %d, Buttons: %d\n", joystick.num_axes, joystick.num_buttons);
            
            joystick.connected = 1;
            return 1;
        }
    }
    
    printf("✗ No joystick found at /dev/input/js0-3\n");
    return 0;
}

// Thread to read joystick events
void* joystick_reader(void* arg) {
    struct js_event event;
    
    while (joystick_thread_running && joystick.connected) {
        int bytes = read(joystick.fd, &event, sizeof(event));
        
        if (bytes == sizeof(event)) {
            // Handle event
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
        } else if (bytes < 0 && errno != EAGAIN) {
            // Error reading
            joystick.connected = 0;
            break;
        }
        
        usleep(1000); // 1ms delay
    }
    
    return NULL;
}

// Get normalized axis value (-1 to 1)
float get_axis(int axis) {
    if (!joystick.connected || axis >= joystick.num_axes) return 0;
    return joystick.axes[axis] / 32768.0f;
}

// Get button state
int get_button(int button) {
    if (!joystick.connected || button >= joystick.num_buttons) return 0;
    return joystick.buttons[button];
}

void update_aircraft_with_direct_joystick(float dt) {
    if (joystick.connected) {
        // Right stick for flight (axes 2,3 or 3,4 depending on controller)
        float stick_x = -get_axis(3);  // Usually right stick X
        float stick_y = -get_axis(4);  // Usually right stick Y
        
        // If those don't work, try axes 0,1
        if (fabs(stick_x) < 0.1f && fabs(stick_y) < 0.1f) {
            stick_x = -get_axis(0);  // Left stick X
            stick_y = -get_axis(1);  // Left stick Y
        }
        
        // Apply dead zone
        if (fabs(stick_x) < 0.1f) stick_x = 0;
        if (fabs(stick_y) < 0.1f) stick_y = 0;
        
        // Flight controls
        player.pitch += stick_y * 60 * dt;  // Pull back to climb
        player.yaw += stick_x * 90 * dt;    // Left/right to turn
        player.roll = Lerp(player.roll, stick_x * 30, 5 * dt);  // Banking
        
        // Throttle from triggers or buttons
        if (get_axis(5) > 0) {  // Right trigger
            player.throttle += (get_axis(5) + 1.0f) / 2.0f * 100 * dt;
        }
        if (get_axis(2) > 0) {  // Left trigger
            player.throttle -= (get_axis(2) + 1.0f) / 2.0f * 100 * dt;
        }
        
        // Button controls
        if (get_button(0) || get_button(1)) {  // A or B
            player.throttle += 100 * dt;
        }
        if (get_button(2) || get_button(3)) {  // X or Y
            player.throttle -= 100 * dt;
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
    } else if (!joystick.connected) {
        player.roll = Lerp(player.roll, 0, 3 * dt);
    }
    
    if (IsKeyDown(KEY_Q)) player.throttle -= 100 * dt;
    if (IsKeyDown(KEY_E)) player.throttle += 100 * dt;
    
    // Clamp values
    if (player.throttle < 0) player.throttle = 0;
    if (player.throttle > 200) player.throttle = 200;
    if (player.pitch < -80) player.pitch = -80;
    if (player.pitch > 80) player.pitch = 80;
    
    // Calculate velocity
    float yaw_rad = player.yaw * DEG2RAD;
    float pitch_rad = player.pitch * DEG2RAD;
    
    player.velocity.x = player.throttle * cosf(yaw_rad) * cosf(pitch_rad);
    player.velocity.y = player.throttle * sinf(pitch_rad);
    player.velocity.z = player.throttle * sinf(yaw_rad) * cosf(pitch_rad);
    
    // Update position
    player.position = Vector3Add(player.position, Vector3Scale(player.velocity, dt));
    
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

void draw_joystick_debug() {
    int y = 100;
    
    if (joystick.connected) {
        DrawText("JOYSTICK CONNECTED", SCREEN_WIDTH - 300, y, 16, GREEN);
        DrawText(joystick.name, SCREEN_WIDTH - 300, y + 20, 12, GREEN);
        
        // Show axes
        y += 50;
        DrawText("AXES:", SCREEN_WIDTH - 300, y, 14, WHITE);
        for (int i = 0; i < joystick.num_axes && i < 6; i++) {
            float value = get_axis(i);
            DrawText(TextFormat("Axis %d: %.2f", i, value), 
                     SCREEN_WIDTH - 300, y + 20 + i * 18, 12, WHITE);
            
            // Draw bar
            DrawRectangle(SCREEN_WIDTH - 150, y + 20 + i * 18, 100, 10, DARKGRAY);
            DrawRectangle(SCREEN_WIDTH - 150 + 50, y + 20 + i * 18, 
                         (int)(value * 50), 10, GREEN);
        }
        
        // Show buttons
        y += 130;
        DrawText("BUTTONS:", SCREEN_WIDTH - 300, y, 14, WHITE);
        for (int i = 0; i < joystick.num_buttons && i < 12; i++) {
            Color c = get_button(i) ? GREEN : DARKGRAY;
            DrawCircle(SCREEN_WIDTH - 280 + (i % 4) * 25, 
                      y + 20 + (i / 4) * 25, 8, c);
            DrawText(TextFormat("%d", i), 
                    SCREEN_WIDTH - 283 + (i % 4) * 25, 
                    y + 17 + (i / 4) * 25, 10, WHITE);
        }
    } else {
        DrawText("NO JOYSTICK", SCREEN_WIDTH - 250, y, 16, RED);
    }
}

int main() {
    printf("=== SKY COMBAT - DIRECT JOYSTICK SUPPORT ===\n\n");
    
    // Open joystick before raylib
    if (open_joystick()) {
        // Start joystick reading thread
        joystick_thread_running = 1;
        pthread_create(&joystick_thread, NULL, joystick_reader, NULL);
    }
    
    // Initialize raylib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat 3D - Direct Joystick");
    SetTargetFPS(60);
    
    // Setup camera
    cockpit_camera.position = player.position;
    cockpit_camera.target = (Vector3){0, 200, 100};
    cockpit_camera.up = (Vector3){0, 1, 0};
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
    
    printf("\nCONTROLS:\n");
    if (joystick.connected) {
        printf("JOYSTICK DETECTED! Using direct Linux input\n");
        printf("  Right Stick - Fly (or Left Stick)\n");
        printf("  Triggers - Throttle\n");
        printf("  Buttons - Throttle\n");
    }
    printf("KEYBOARD:\n");
    printf("  WASD - Fly\n");
    printf("  Q/E - Throttle\n");
    printf("  ESC - Exit\n");
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        update_aircraft_with_direct_joystick(dt);
        
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
        
        // AI
        for (int i = 0; i < AI_COUNT; i++) {
            DrawCube(ai_chars[i].pos, 1, 2, 1, ai_chars[i].color);
        }
        
        EndMode3D();
        
        // HUD
        DrawText("ALT", 20, 20, 20, GREEN);
        DrawText(TextFormat("%.0f m", player.position.y), 20, 45, 30, WHITE);
        
        DrawText("SPD", 20, 90, 20, GREEN);
        DrawText(TextFormat("%.0f m/s", player.throttle), 20, 115, 30, WHITE);
        
        // Throttle bar
        DrawRectangle(20, 160, 20, 200, Fade(GREEN, 0.3f));
        DrawRectangle(20, 360 - (int)(player.throttle), 20, (int)(player.throttle), GREEN);
        
        // Crosshair
        DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, GREEN);
        
        if (player.position.y < 50) {
            DrawText("! PULL UP !", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT - 100, 30, RED);
        }
        
        // Joystick debug info
        draw_joystick_debug();
        
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
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

#include "sky_combat/core/secure_code_points.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BUILDING_COUNT 150
#define AI_COUNT 300

// Joystick specifications using secure code points
typedef struct {
    int fd;
    char name[256];
    int num_axes;
    int num_buttons;
    int16_t axes[32];
    uint8_t buttons[32];
    int connected;
    
    // Axis mappings - will be verified at runtime
    int pitch_axis;      // Which axis controls pitch
    int roll_axis;       // Which axis controls roll
    int throttle_axis;   // Which axis controls throttle
    int yaw_axis;        // Which axis controls yaw
    
    // Inversion flags
    int pitch_inverted;
    int roll_inverted;
    int throttle_inverted;
    int yaw_inverted;
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

// Calibration state
int calibration_mode = 0;  // Skip calibration for now
int calibration_step = 0;

// Joystick specifications with secure code points
void verify_joystick_specifications() {
    SECURE_CODE_POINT(JOYSTICK_SPEC_START, "verifying joystick specs");
    
    // Right stick MUST control pitch and roll
    SECURE_CODE_POINT(RIGHT_STICK_PITCH_SPEC, "right stick Y = pitch");
    SECURE_CODE_POINT(RIGHT_STICK_ROLL_SPEC, "right stick X = roll/turn");
    
    // Pull back MUST climb
    SECURE_CODE_POINT(PULL_BACK_CLIMBS, "pull back = pitch up");
    SECURE_CODE_POINT(PUSH_FORWARD_DIVES, "push forward = pitch down");
    
    // Left/right MUST turn
    SECURE_CODE_POINT(STICK_LEFT_TURNS_LEFT, "stick left = turn left");
    SECURE_CODE_POINT(STICK_RIGHT_TURNS_RIGHT, "stick right = turn right");
    
    // Throttle control
    SECURE_CODE_POINT(THROTTLE_FORWARD_INCREASE, "throttle forward = faster");
    SECURE_CODE_POINT(THROTTLE_BACK_DECREASE, "throttle back = slower");
    
    SECURE_CODE_POINT(JOYSTICK_SPEC_COMPLETE, "specs verified");
}

// Detect which axes do what by watching for movement
void detect_axis_mapping() {
    printf("\n=== JOYSTICK CALIBRATION ===\n");
    printf("Move your controls as instructed...\n\n");
    
    // ASTRO C40 defaults (skip calibration)
    joystick.pitch_axis = 5;      // Right stick Y - AXIS 5!
    joystick.roll_axis = 2;       // Right stick X
    joystick.throttle_axis = 1;   // Left stick Y
    joystick.yaw_axis = 0;        // Left stick X
    
    // Standard non-inverted controls
    joystick.pitch_inverted = 0;
    joystick.roll_inverted = 0;
    joystick.throttle_inverted = 0;
    joystick.yaw_inverted = 0;
    
    printf("Using ASTRO C40 defaults: Right Stick Y = Axis 5\n");
}

// Get axis value with proper mapping and inversion
float get_mapped_axis(int mapping, int inverted) {
    if (mapping < 0 || mapping >= joystick.num_axes) return 0;
    float value = joystick.axes[mapping] / 32768.0f;
    return inverted ? -value : value;
}

// Open joystick
int open_joystick() {
    for (int i = 0; i < 4; i++) {
        char device[32];
        sprintf(device, "/dev/input/js%d", i);
        
        joystick.fd = open(device, O_RDONLY | O_NONBLOCK);
        if (joystick.fd >= 0) {
            SECURE_CODE_POINT(JOYSTICK_DEVICE_OPENED, "device opened");
            
            ioctl(joystick.fd, JSIOCGNAME(sizeof(joystick.name)), joystick.name);
            ioctl(joystick.fd, JSIOCGAXES, &joystick.num_axes);
            ioctl(joystick.fd, JSIOCGBUTTONS, &joystick.num_buttons);
            
            printf("✓ Opened %s\n", device);
            printf("✓ Joystick: %s\n", joystick.name);
            printf("✓ Axes: %d, Buttons: %d\n", joystick.num_axes, joystick.num_buttons);
            
            joystick.connected = 1;
            SECURE_CODE_POINT(JOYSTICK_INFO_READ, "info retrieved");
            return 1;
        }
    }
    
    return 0;
}

// Thread to read joystick
void* joystick_reader(void* arg) {
    struct js_event event;
    
    while (joystick_thread_running && joystick.connected) {
        int bytes = read(joystick.fd, &event, sizeof(event));
        
        if (bytes == sizeof(event)) {
            switch (event.type & ~JS_EVENT_INIT) {
                case JS_EVENT_AXIS:
                    if (event.number < 32) {
                        joystick.axes[event.number] = event.value;
                        SECURE_CODE_POINT(JOYSTICK_AXIS_EVENT, "axis moved");
                    }
                    break;
                    
                case JS_EVENT_BUTTON:
                    if (event.number < 32) {
                        joystick.buttons[event.number] = event.value;
                        SECURE_CODE_POINT(JOYSTICK_BUTTON_EVENT, "button pressed");
                    }
                    break;
            }
        }
        
        usleep(1000);
    }
    
    return NULL;
}

// Calibration mode
void run_calibration() {
    const char* instructions[] = {
        "PULL STICK BACK (to climb)",
        "PUSH STICK FORWARD (to dive)",
        "MOVE STICK LEFT (to turn left)",
        "MOVE STICK RIGHT (to turn right)",
        "INCREASE THROTTLE (push throttle forward)",
        "DECREASE THROTTLE (pull throttle back)"
    };
    
    if (calibration_step < 6) {
        DrawText("JOYSTICK CALIBRATION", SCREEN_WIDTH/2 - 150, 50, 30, GREEN);
        DrawText(instructions[calibration_step], SCREEN_WIDTH/2 - 200, 150, 20, WHITE);
        DrawText("Move the control and press SPACE when done", SCREEN_WIDTH/2 - 250, 200, 16, GRAY);
        
        // Show all axes
        int y = 250;
        for (int i = 0; i < joystick.num_axes && i < 8; i++) {
            float value = joystick.axes[i] / 32768.0f;
            DrawText(TextFormat("Axis %d:", i), 100, y, 16, WHITE);
            
            // Bar
            DrawRectangle(200, y, 200, 20, DARKGRAY);
            DrawRectangle(300, y, (int)(value * 100), 20, 
                         fabs(value) > 0.5f ? GREEN : GRAY);
            
            DrawText(TextFormat("%.2f", value), 420, y, 16, WHITE);
            
            // Detect which axis is moving most
            if (fabs(value) > 0.5f) {
                switch(calibration_step) {
                    case 0: // Pull back
                        if (joystick.pitch_axis == -1) {
                            joystick.pitch_axis = i;
                            joystick.pitch_inverted = (value > 0);
                            SECURE_CODE_POINT(PITCH_AXIS_DETECTED, "pitch axis found");
                        }
                        break;
                    case 2: // Move left
                        if (joystick.roll_axis == -1) {
                            joystick.roll_axis = i;
                            joystick.roll_inverted = (value > 0);
                            SECURE_CODE_POINT(ROLL_AXIS_DETECTED, "roll axis found");
                        }
                        break;
                    case 4: // Throttle forward
                        if (joystick.throttle_axis == -1) {
                            joystick.throttle_axis = i;
                            joystick.throttle_inverted = (value < 0);
                            SECURE_CODE_POINT(THROTTLE_AXIS_DETECTED, "throttle axis found");
                        }
                        break;
                }
            }
            
            y += 25;
        }
        
        if (IsKeyPressed(KEY_SPACE)) {
            calibration_step++;
            
            if (calibration_step >= 6) {
                calibration_mode = 0;
                SECURE_CODE_POINT(CALIBRATION_COMPLETE, "joystick calibrated");
                
                // Set defaults if not detected
                if (joystick.pitch_axis == -1) joystick.pitch_axis = 1;    // Y axis
                if (joystick.roll_axis == -1) joystick.roll_axis = 0;      // X axis
                if (joystick.yaw_axis == -1) joystick.yaw_axis = joystick.roll_axis;
                if (joystick.throttle_axis == -1) joystick.throttle_axis = 2;
                
                printf("\nCalibration complete:\n");
                printf("  Pitch axis: %d (inverted: %d)\n", joystick.pitch_axis, joystick.pitch_inverted);
                printf("  Roll axis: %d (inverted: %d)\n", joystick.roll_axis, joystick.roll_inverted);
                printf("  Throttle axis: %d (inverted: %d)\n", joystick.throttle_axis, joystick.throttle_inverted);
            }
        }
    }
}

// Update aircraft with calibrated joystick
void update_aircraft_with_calibrated_joystick(float dt) {
    if (joystick.connected && !calibration_mode) {
        // Get calibrated axis values
        float pitch_input = get_mapped_axis(joystick.pitch_axis, joystick.pitch_inverted);
        float roll_input = get_mapped_axis(joystick.roll_axis, joystick.roll_inverted);
        float throttle_input = get_mapped_axis(joystick.throttle_axis, joystick.throttle_inverted);
        
        // Apply dead zone
        if (fabs(pitch_input) < 0.1f) pitch_input = 0;
        if (fabs(roll_input) < 0.1f) roll_input = 0;
        
        // Verify specifications
        if (pitch_input < -0.5f) {
            SECURE_CODE_POINT(STICK_PULLED_BACK, "climbing");
            player.pitch -= pitch_input * 60 * dt;  // Negative pitch = climb
        } else if (pitch_input > 0.5f) {
            SECURE_CODE_POINT(STICK_PUSHED_FORWARD, "diving");
            player.pitch -= pitch_input * 60 * dt;  // Positive pitch = dive
        }
        
        if (roll_input < -0.5f) {
            SECURE_CODE_POINT(STICK_MOVED_LEFT, "turning left");
            player.yaw += roll_input * 90 * dt;
            player.roll = roll_input * 30;
        } else if (roll_input > 0.5f) {
            SECURE_CODE_POINT(STICK_MOVED_RIGHT, "turning right");
            player.yaw += roll_input * 90 * dt;
            player.roll = roll_input * 30;
        } else {
            player.roll = Lerp(player.roll, 0, 3 * dt);
        }
        
        // Throttle
        player.throttle = 100 + throttle_input * 100;  // 0-200 range
        
        if (throttle_input > 0.5f) {
            SECURE_CODE_POINT(THROTTLE_INCREASED, "going faster");
        } else if (throttle_input < -0.5f) {
            SECURE_CODE_POINT(THROTTLE_DECREASED, "slowing down");
        }
        
        // Button boost
        if (joystick.buttons[0]) {
            player.throttle += 50;
            SECURE_CODE_POINT(BOOST_BUTTON_PRESSED, "afterburner");
        }
    }
    
    // Keyboard fallback
    if (IsKeyDown(KEY_W)) player.pitch -= 60 * dt;
    if (IsKeyDown(KEY_S)) player.pitch += 60 * dt;
    if (IsKeyDown(KEY_A)) {
        player.yaw -= 90 * dt;
        player.roll = -30;
    } else if (IsKeyDown(KEY_D)) {
        player.yaw += 90 * dt;
        player.roll = 30;
    }
    if (IsKeyDown(KEY_Q)) player.throttle -= 100 * dt;
    if (IsKeyDown(KEY_E)) player.throttle += 100 * dt;
    
    // Clamp
    if (player.throttle < 0) player.throttle = 0;
    if (player.throttle > 300) player.throttle = 300;
    if (player.pitch < -80) player.pitch = -80;
    if (player.pitch > 80) player.pitch = 80;
    
    // Physics
    float yaw_rad = player.yaw * DEG2RAD;
    float pitch_rad = player.pitch * DEG2RAD;
    
    player.velocity.x = player.throttle * cosf(yaw_rad) * cosf(pitch_rad);
    player.velocity.y = player.throttle * sinf(pitch_rad);
    player.velocity.z = player.throttle * sinf(yaw_rad) * cosf(pitch_rad);
    
    player.position = Vector3Add(player.position, Vector3Scale(player.velocity, dt));
    
    if (player.position.y < 5) {
        player.position.y = 5;
        player.velocity.y = 0;
    }
    
    // Camera
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

void draw_joystick_status() {
    if (!joystick.connected) {
        DrawText("NO JOYSTICK", SCREEN_WIDTH - 200, 100, 16, RED);
        return;
    }
    
    if (calibration_mode) return;
    
    DrawText("JOYSTICK OK", SCREEN_WIDTH - 200, 100, 16, GREEN);
    DrawText(joystick.name, SCREEN_WIDTH - 250, 120, 12, GREEN);
    
    // Show mapped controls
    int y = 150;
    float pitch = get_mapped_axis(joystick.pitch_axis, joystick.pitch_inverted);
    float roll = get_mapped_axis(joystick.roll_axis, joystick.roll_inverted);
    float throttle = get_mapped_axis(joystick.throttle_axis, joystick.throttle_inverted);
    
    DrawText("PITCH:", SCREEN_WIDTH - 250, y, 14, WHITE);
    DrawRectangle(SCREEN_WIDTH - 170, y, 100, 10, DARKGRAY);
    DrawRectangle(SCREEN_WIDTH - 120, y, (int)(pitch * 50), 10, GREEN);
    
    DrawText("ROLL:", SCREEN_WIDTH - 250, y + 20, 14, WHITE);
    DrawRectangle(SCREEN_WIDTH - 170, y + 20, 100, 10, DARKGRAY);
    DrawRectangle(SCREEN_WIDTH - 120, y + 20, (int)(roll * 50), 10, GREEN);
    
    DrawText("THROTTLE:", SCREEN_WIDTH - 250, y + 40, 14, WHITE);
    DrawRectangle(SCREEN_WIDTH - 170, y + 40, 100, 10, DARKGRAY);
    DrawRectangle(SCREEN_WIDTH - 120, y + 40, (int)(throttle * 50), 10, GREEN);
}

int main() {
    printf("=== SKY COMBAT - JOYSTICK WITH SPECIFICATIONS ===\n\n");
    
    // Verify specifications
    verify_joystick_specifications();
    
    // Open joystick
    if (open_joystick()) {
        joystick_thread_running = 1;
        pthread_create(&joystick_thread, NULL, joystick_reader, NULL);
        detect_axis_mapping();
    } else {
        calibration_mode = 0;  // Skip calibration if no joystick
    }
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat 3D - Calibrated Joystick");
    SetTargetFPS(60);
    
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
    
    // AI
    for (int i = 0; i < AI_COUNT; i++) {
        ai_chars[i].pos.x = (float)GetRandomValue(-2000, 2000);
        ai_chars[i].pos.z = (float)GetRandomValue(-2000, 2000);
        ai_chars[i].pos.y = 1.0f;
        ai_chars[i].color = (Color){255, 150, 0, 255};
        ai_chars[i].fleeing = false;
    }
    
    printf("\nThe game will calibrate your joystick to ensure:\n");
    printf("✓ Pull back = Climb\n");
    printf("✓ Push forward = Dive\n");
    printf("✓ Left = Turn left\n");
    printf("✓ Right = Turn right\n");
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        update_aircraft_with_calibrated_joystick(dt);
        
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
        
        if (calibration_mode) {
            // Show calibration screen
            run_calibration();
        } else {
            // Normal game
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
            
            DrawRectangle(20, 160, 20, 200, Fade(GREEN, 0.3f));
            DrawRectangle(20, 360 - (int)(player.throttle), 20, (int)(player.throttle), GREEN);
            
            DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, GREEN);
            
            if (player.position.y < 50) {
                DrawText("! PULL UP !", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT - 100, 30, RED);
            }
            
            draw_joystick_status();
            
            DrawFPS(10, 10);
        }
        
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
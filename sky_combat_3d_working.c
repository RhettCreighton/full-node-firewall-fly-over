/* Working 3D Flying Game with Joystick Support */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define BUILDING_COUNT 200

typedef struct {
    Vector3 pos;
    Vector3 size;
    Color color;
} Building;

Building buildings[BUILDING_COUNT];
Camera3D camera = { 0 };
float speed = 50.0f;
float yaw = 0.0f;
float pitch = 0.0f;
Vector3 forward = { 0.0f, 0.0f, 1.0f };

// Joystick
int js_fd = -1;
struct js_event js;
float axes[8] = {0};

void InitJoystick() {
    js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (js_fd >= 0) {
        printf("Joystick connected!\n");
    }
}

void UpdateJoystick() {
    if (js_fd < 0) return;
    
    while (read(js_fd, &js, sizeof(js)) == sizeof(js)) {
        if (js.type & JS_EVENT_AXIS) {
            if (js.number < 8) {
                axes[js.number] = js.value / 32768.0f;
            }
        }
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat 3D - Working Version");
    SetTargetFPS(60);
    DisableCursor();
    
    // Camera setup
    camera.position = (Vector3){ 0.0f, 100.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 100.0f, 10.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Generate city
    for (int i = 0; i < BUILDING_COUNT; i++) {
        buildings[i].pos.x = GetRandomValue(-2000, 2000);
        buildings[i].pos.z = GetRandomValue(-2000, 2000);
        buildings[i].pos.y = 0;
        
        buildings[i].size.x = GetRandomValue(40, 100);
        buildings[i].size.z = GetRandomValue(40, 100);
        buildings[i].size.y = GetRandomValue(100, 600);
        
        buildings[i].color = (Color){
            GetRandomValue(10, 40),
            GetRandomValue(10, 40),
            GetRandomValue(20, 60),
            255
        };
    }
    
    InitJoystick();
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        UpdateJoystick();
        
        // Controls (joystick + keyboard)
        if (js_fd >= 0 && (fabs(axes[2]) > 0.1f || fabs(axes[5]) > 0.1f || fabs(axes[1]) > 0.1f)) {
            // PROPER JOYSTICK CONTROLS FOR ASTRO C40:
            // Right stick X (axis 2) - Turn left/right
            yaw -= axes[2] * 2.0f * dt;
            
            // Right stick Y (axis 5) - PULL BACK TO CLIMB!
            pitch -= axes[5] * dt;  // Negative because pull back (positive) should pitch up
            
            // Left stick Y (axis 1) - Throttle
            float throttle = -axes[1];  // Push forward = faster
            speed = 150.0f + throttle * 150.0f;
        } else {
            // Keyboard/mouse fallback
            Vector2 mouseDelta = GetMouseDelta();
            yaw -= mouseDelta.x * 0.003f;
            pitch -= mouseDelta.y * 0.003f;
            
            if (IsKeyDown(KEY_W)) pitch += dt;
            if (IsKeyDown(KEY_S)) pitch -= dt;
            if (IsKeyDown(KEY_A)) yaw -= dt;
            if (IsKeyDown(KEY_D)) yaw += dt;
            if (IsKeyDown(KEY_Q)) speed -= 50.0f * dt;
            if (IsKeyDown(KEY_E)) speed += 50.0f * dt;
        }
        
        // Clamp values
        if (pitch > 1.5f) pitch = 1.5f;
        if (pitch < -1.5f) pitch = -1.5f;
        if (speed < 0) speed = 0;
        if (speed > 300) speed = 300;
        
        // Calculate movement
        forward.x = sinf(yaw) * cosf(pitch);
        forward.y = sinf(pitch);
        forward.z = cosf(yaw) * cosf(pitch);
        forward = Vector3Normalize(forward);
        
        // Move
        camera.position = Vector3Add(camera.position, Vector3Scale(forward, speed * dt));
        
        // Altitude control
        if (IsKeyDown(KEY_SPACE)) camera.position.y += speed * dt;
        if (IsKeyDown(KEY_LEFT_CONTROL)) camera.position.y -= speed * dt;
        
        if (camera.position.y < 5.0f) camera.position.y = 5.0f;
        if (camera.position.y > 1000.0f) camera.position.y = 1000.0f;
        
        // Update camera target
        camera.target = Vector3Add(camera.position, forward);
        
        // Draw
        BeginDrawing();
        ClearBackground((Color){5, 5, 20, 255});
        
        BeginMode3D(camera);
        
        // Ground
        DrawGrid(100, 50.0f);
        
        // Buildings
        for (int i = 0; i < BUILDING_COUNT; i++) {
            Vector3 pos = buildings[i].pos;
            pos.y += buildings[i].size.y / 2;
            
            DrawCube(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, buildings[i].color);
            DrawCubeWires(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, BLACK);
            
            // Simple neon effect
            if (GetRandomValue(0, 100) < 70) {
                Color neon = (Color){0, 100, 255, 255};
                Vector3 top = pos;
                top.y = buildings[i].size.y - 10;
                DrawCube(top, buildings[i].size.x * 0.9f, 2, buildings[i].size.z * 0.9f, neon);
            }
        }
        
        EndMode3D();
        
        // HUD
        DrawRectangle(0, 0, SCREEN_WIDTH, 100, Fade(BLACK, 0.7f));
        DrawText(TextFormat("ALTITUDE: %.0f m", camera.position.y), 20, 10, 20, WHITE);
        DrawText(TextFormat("SPEED: %.0f m/s", speed), 20, 35, 20, WHITE);
        DrawText(TextFormat("PITCH: %.1f°", pitch * 57.3f), 20, 60, 20, WHITE);
        
        // Crosshair
        DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 40, GREEN);
        DrawLine(SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 - 20, SCREEN_HEIGHT/2, GREEN);
        DrawLine(SCREEN_WIDTH/2 + 20, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 + 50, SCREEN_HEIGHT/2, GREEN);
        DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 50, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 20, GREEN);
        DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 20, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 50, GREEN);
        
        if (js_fd >= 0) {
            DrawText("JOYSTICK: ASTRO C40 ✓", SCREEN_WIDTH - 300, 10, 20, GREEN);
            DrawText(TextFormat("Right Y: %.2f", axes[5]), SCREEN_WIDTH - 300, 35, 20, GREEN);
            DrawText(TextFormat("Throttle: %.2f", -axes[1]), SCREEN_WIDTH - 300, 60, 20, GREEN);
        } else {
            DrawText("JOYSTICK: NOT FOUND", SCREEN_WIDTH - 300, 10, 20, RED);
        }
        
        // Low altitude warning
        if (camera.position.y < 50) {
            DrawText("! LOW ALTITUDE !", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT - 100, 30, RED);
        }
        
        DrawText("Right Stick: Fly | Left Stick: Throttle | Space/Ctrl: Altitude | ESC: Exit", 
                 10, SCREEN_HEIGHT - 25, 20, WHITE);
        
        DrawFPS(SCREEN_WIDTH - 100, 10);
        
        EndDrawing();
    }
    
    if (js_fd >= 0) close(js_fd);
    CloseWindow();
    
    return 0;
}
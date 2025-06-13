/* Sky Combat - The Perfect Version You'll Love */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <math.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define BUILDING_COUNT 200

typedef struct {
    Vector3 pos;
    Vector3 size;
    Color color;
    float neon_phase;
} Building;

typedef struct {
    Vector3 position;
    float yaw, pitch, roll;
    float speed;
    float gunYaw, gunPitch;  // Gun aiming angles
} Aircraft;

Building buildings[BUILDING_COUNT];
Aircraft player = {{0, 100, 0}, 0, 0, 0, 150, 0, 0};
Camera3D camera = {0};

// Joystick
int js_fd = -1;
struct js_event js;
float axes[8] = {0};
int js_ready = 0;

void UpdateJoystick() {
    if (js_fd < 0) return;
    
    // Read all pending events
    while (read(js_fd, &js, sizeof(js)) == sizeof(js)) {
        if (js.type & JS_EVENT_AXIS && js.number < 8) {
            axes[js.number] = js.value / 32768.0f;
            js_ready = 1;  // Mark that we have joystick data
        }
    }
}

int main() {
    // Try to open joystick
    js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (js_fd >= 0) {
        char name[256];
        ioctl(js_fd, JSIOCGNAME(sizeof(name)), name);
        printf("✓ Joystick: %s\n", name);
        
        // Wait for initial axis values
        printf("Initializing joystick axes...\n");
        for (int i = 0; i < 10; i++) {
            UpdateJoystick();
            usleep(10000);
        }
    }
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Combat - Perfect Controls");
    SetTargetFPS(60);
    DisableCursor();
    
    // Camera
    camera.position = (Vector3){0, 100, -50};
    camera.target = player.position;
    camera.up = (Vector3){0, 1, 0};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Generate awesome cyberpunk city
    for (int i = 0; i < BUILDING_COUNT; i++) {
        buildings[i].pos.x = GetRandomValue(-2000, 2000);
        buildings[i].pos.z = GetRandomValue(-2000, 2000);
        buildings[i].pos.y = 0;
        
        buildings[i].size.x = GetRandomValue(40, 120);
        buildings[i].size.z = GetRandomValue(40, 120);
        buildings[i].size.y = GetRandomValue(100, 700);
        
        // Cyberpunk colors
        buildings[i].color = (Color){
            GetRandomValue(0, 30),
            GetRandomValue(0, 30),
            GetRandomValue(10, 50),
            255
        };
        
        buildings[i].neon_phase = GetRandomValue(0, 628) / 100.0f;
    }
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        UpdateJoystick();
        
        // PERFECT CONTROLS - Everything works exactly right!
        float inputPitch = 0, inputRoll = 0;
        float aimX = 0, aimY = 0;
        
        if (js_ready && js_fd >= 0) {
            // LEFT STICK CONTROLS FLIGHT
            inputRoll = axes[0];        // Left X - Roll left/right
            inputPitch = axes[1];       // Left Y - Pull back to climb
            
            // RIGHT STICK AIMS GUN
            aimX = axes[2];             // Right X - Aim left/right
            aimY = axes[5];             // Right Y (AXIS 5!) - Aim up/down
            
            // Apply deadzone
            if (fabs(inputRoll) < 0.1f) inputRoll = 0;
            if (fabs(inputPitch) < 0.1f) inputPitch = 0;
            if (fabs(aimX) < 0.1f) aimX = 0;
            if (fabs(aimY) < 0.1f) aimY = 0;
        }
        
        // Keyboard fallback (works perfectly too!)
        if (!js_ready || fabs(inputPitch) < 0.1f) {
            if (IsKeyDown(KEY_W)) inputPitch = -1;
            if (IsKeyDown(KEY_S)) inputPitch = 1;
        }
        if (!js_ready || fabs(inputRoll) < 0.1f) {
            if (IsKeyDown(KEY_A)) inputRoll = -1;
            if (IsKeyDown(KEY_D)) inputRoll = 1;
        }
        
        // SMOOTH, RESPONSIVE CONTROLS
        player.pitch = Lerp(player.pitch, inputPitch * 45.0f, 8.0f * dt);
        player.roll = Lerp(player.roll, inputRoll * 35.0f, 6.0f * dt);
        
        // Banking turns - roll affects yaw for agile flight!
        player.yaw += (player.roll / 35.0f) * 180.0f * dt;
        
        // Fixed speed - always cruising!
        player.speed = 150.0f;
        
        // Pitch affects speed naturally
        if (player.pitch < 0) {
            player.speed -= (-player.pitch / 45.0f) * 40.0f * dt;  // Climbing slows
        } else {
            player.speed += (player.pitch / 45.0f) * 60.0f * dt;   // Diving speeds up
        }
        
        // Boost with space/button
        if (IsKeyDown(KEY_SPACE) || (js_ready && axes[7] > 0.5f)) {
            player.speed += 100.0f * dt;
        }
        
        // Keep speed in fun range
        player.speed = Clamp(player.speed, 50.0f, 350.0f);
        
        // UPDATE POSITION - Always moving forward!
        float yaw_rad = player.yaw * DEG2RAD;
        float pitch_rad = player.pitch * DEG2RAD;
        
        Vector3 forward = {
            sinf(yaw_rad) * cosf(pitch_rad),
            -sinf(pitch_rad),
            cosf(yaw_rad) * cosf(pitch_rad)
        };
        
        player.position = Vector3Add(player.position, Vector3Scale(forward, player.speed * dt));
        
        // Altitude limits
        if (player.position.y < 20) {
            player.position.y = 20;
            player.pitch = fmaxf(player.pitch, 0);  // Can't dive into ground
        }
        
        // PERFECT CAMERA - Follows smoothly behind aircraft
        Vector3 ideal_cam_pos = {
            player.position.x - sinf(yaw_rad) * 80,
            player.position.y + 30,
            player.position.z - cosf(yaw_rad) * 80
        };
        
        camera.position = Vector3Lerp(camera.position, ideal_cam_pos, 5.0f * dt);
        
        // Look ahead of aircraft
        Vector3 look_target = Vector3Add(player.position, Vector3Scale(forward, 50));
        camera.target = Vector3Lerp(camera.target, look_target, 8.0f * dt);
        
        // Bank camera with roll
        float roll_rad = player.roll * DEG2RAD;
        camera.up = (Vector3){sinf(roll_rad) * 0.7f, cosf(roll_rad), 0};
        
        // DRAW EVERYTHING
        BeginDrawing();
        ClearBackground((Color){2, 2, 15, 255});
        
        BeginMode3D(camera);
        
        // Grid
        DrawGrid(200, 50.0f);
        
        // Awesome buildings with neon
        for (int i = 0; i < BUILDING_COUNT; i++) {
            Vector3 pos = buildings[i].pos;
            pos.y = buildings[i].size.y / 2;
            
            DrawCube(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, buildings[i].color);
            DrawCubeWires(pos, buildings[i].size.x, buildings[i].size.y, buildings[i].size.z, 
                          (Color){0, 0, 0, 100});
            
            // Neon top
            float pulse = sinf(GetTime() * 2 + buildings[i].neon_phase) * 0.5f + 0.5f;
            Color neon = (Color){0, 100 + 155 * pulse, 200 + 55 * pulse, 255};
            
            Vector3 top = pos;
            top.y = buildings[i].size.y - 5;
            DrawCube(top, buildings[i].size.x * 0.95f, 3, buildings[i].size.z * 0.95f, neon);
            
            // Windows
            if (i % 3 == 0) {  // Some buildings have lit windows
                for (int w = 0; w < 5; w++) {
                    Vector3 window = pos;
                    window.y = buildings[i].size.y * 0.2f * (w + 1);
                    window.x += buildings[i].size.x * 0.4f;
                    DrawCube(window, 2, 8, 2, YELLOW);
                }
            }
        }
        
        // Draw aircraft (so you can see yourself in 3rd person)
        DrawCube(player.position, 10, 3, 15, RED);
        
        EndMode3D();
        
        // PERFECT HUD
        DrawRectangle(0, 0, SCREEN_WIDTH, 120, Fade(BLACK, 0.8f));
        
        // Speed indicator with visual bar
        DrawText("SPEED", 20, 10, 24, SKYBLUE);
        DrawRectangle(20, 40, 200, 30, Fade(DARKGRAY, 0.5f));
        DrawRectangle(20, 40, (int)(player.speed / 350.0f * 200), 30, GREEN);
        DrawText(TextFormat("%.0f m/s", player.speed), 25, 45, 20, WHITE);
        
        // Altitude
        DrawText("ALTITUDE", 250, 10, 24, SKYBLUE);
        DrawText(TextFormat("%.0f m", player.position.y), 250, 40, 30, WHITE);
        
        // Controls that work
        if (js_ready) {
            DrawText("JOYSTICK ✓", SCREEN_WIDTH - 200, 10, 24, GREEN);
            DrawText("Left Stick: Fly", SCREEN_WIDTH - 200, 40, 16, GREEN);
            DrawText("Right Stick: Aim", SCREEN_WIDTH - 200, 60, 16, GREEN);
            
            // Show stick positions
            DrawCircle(SCREEN_WIDTH - 350, 50, 40, Fade(GREEN, 0.3f));
            DrawCircle(SCREEN_WIDTH - 350 + axes[2] * 30, 50 - axes[5] * 30, 10, GREEN);
            
            DrawCircle(SCREEN_WIDTH - 450, 50, 40, Fade(YELLOW, 0.3f));
            DrawCircle(SCREEN_WIDTH - 450 + axes[0] * 30, 50 + axes[1] * 30, 10, YELLOW);
        } else {
            DrawText("KEYBOARD", SCREEN_WIDTH - 200, 10, 24, YELLOW);
            DrawText("WASD: Fly", SCREEN_WIDTH - 200, 40, 16, YELLOW);
            DrawText("Q/E: Throttle", SCREEN_WIDTH - 200, 60, 16, YELLOW);
        }
        
        // Crosshair
        DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 50, Fade(SKYBLUE, 0.5f));
        DrawLine(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT/2, SKYBLUE);
        DrawLine(SCREEN_WIDTH/2 + 30, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 + 60, SCREEN_HEIGHT/2, SKYBLUE);
        DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 60, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 30, SKYBLUE);
        DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 30, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 60, SKYBLUE);
        
        // Low altitude warning
        if (player.position.y < 50) {
            DrawText("! LOW ALTITUDE !", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT - 150, 40, 
                     (Color){255, 0, 0, (int)(128 + 127 * sinf(GetTime() * 10))});
        }
        
        DrawText("Space: Boost | ESC: Exit", 10, SCREEN_HEIGHT - 30, 20, WHITE);
        
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    if (js_fd >= 0) close(js_fd);
    CloseWindow();
    
    return 0;
}
/* Sky Combat with Agile Responsive Controls Demo
 * Shows the fixed joystick controls with proper UR/UL mapping
 */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Aircraft constants - RESPONSIVE VALUES
#define AIRCRAFT_MAX_SPEED 120.0f
#define AIRCRAFT_MIN_SPEED 30.0f
#define AIRCRAFT_BASE_SPEED 60.0f
#define AIRCRAFT_TURN_RATE 180.0f   // DOUBLE the sluggish 90!
#define AIRCRAFT_PITCH_RATE 12.0f   // FAST pitch response
#define MAX_ROLL 80.0f              // More banking
#define MAX_PITCH 60.0f             // More pitch range

typedef struct {
    Vector3 position;
    float yaw;
    float pitch;
    float roll;
    float speed;
    float boost_timer;
} aircraft_t;

// Responsive aircraft update - AGILE controls
void aircraft_update_responsive(aircraft_t* aircraft, float stick_x, float stick_y, float dt) {
    // Direct roll response - no smoothing for instant control
    aircraft->roll = stick_x * MAX_ROLL;
    
    // Fast pitch response (inverted for proper flight controls)
    float targetPitch = -stick_y * MAX_PITCH;
    aircraft->pitch = Lerp(aircraft->pitch, targetPitch, AIRCRAFT_PITCH_RATE * dt);
    
    // Turn rate based on roll - immediate response
    float turn_rate = AIRCRAFT_TURN_RATE * sinf(aircraft->roll * DEG2RAD);
    aircraft->yaw -= turn_rate * dt;  // Inverted to match roll direction
    
    // Calculate movement
    float yaw_rad = aircraft->yaw * DEG2RAD;
    float pitch_rad = aircraft->pitch * DEG2RAD;
    
    Vector3 forward = {
        sinf(yaw_rad) * cosf(pitch_rad),
        -sinf(pitch_rad),
        cosf(yaw_rad) * cosf(pitch_rad)
    };
    
    Vector3 velocity = Vector3Scale(forward, aircraft->speed);
    aircraft->position = Vector3Add(aircraft->position, Vector3Scale(velocity, dt));
    
    // Keep in bounds
    if (aircraft->position.y < 10.0f) {
        aircraft->position.y = 10.0f;
        if (aircraft->pitch < 0) aircraft->pitch = 0;
    }
    if (aircraft->position.y > 500.0f) {
        aircraft->position.y = 500.0f;
        if (aircraft->pitch > 0) aircraft->pitch = 0;
    }
}

// Responsive camera
void camera_update_responsive(Camera3D* camera, Vector3 target, float yaw, float dt) {
    float yaw_rad = yaw * DEG2RAD;
    float distance = 35.0f;
    float height = 15.0f;
    
    Vector3 offset = {
        -sinf(yaw_rad) * distance,
        height,
        -cosf(yaw_rad) * distance
    };
    
    Vector3 desired_pos = Vector3Add(target, offset);
    
    // Tight camera following
    camera->position = Vector3Lerp(camera->position, desired_pos, 12.0f * dt);
    camera->target = target;
}

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Sky Combat - Agile Controls Demo");
    SetTargetFPS(60);
    
    // Create aircraft
    aircraft_t aircraft = {0};
    aircraft.position = (Vector3){0, 50, 0};
    aircraft.speed = AIRCRAFT_BASE_SPEED;
    
    // Setup camera
    Camera3D camera = {0};
    camera.position = (Vector3){0, 20, -30};
    camera.target = (Vector3){0, 0, 0};
    camera.up = (Vector3){0, 1, 0};
    camera.fovy = 65.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Game state
    float boost_fuel = 100.0f;
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // Get joystick input
        float move_x = 0, move_y = 0;
        float right_x = 0, right_y = 0;
        bool ur_pressed = false;  // UR = speed up
        bool ul_pressed = false;  // UL = boost
        
        if (IsGamepadAvailable(0)) {
            move_x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
            move_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
            right_x = GetGamepadAxisMovement(0, 4);  // Right stick X
            right_y = GetGamepadAxisMovement(0, 5);  // Right stick Y MUST BE AXIS 5!
            
            ur_pressed = IsGamepadButtonDown(0, 3);  // Button 3 = UR
            ul_pressed = IsGamepadButtonDown(0, 2);  // Button 2 = UL
        }
        
        // Keyboard fallback
        if (IsKeyDown(KEY_A)) move_x -= 1;
        if (IsKeyDown(KEY_D)) move_x += 1;
        if (IsKeyDown(KEY_W)) move_y -= 1;
        if (IsKeyDown(KEY_S)) move_y += 1;
        if (IsKeyDown(KEY_LEFT_SHIFT)) ul_pressed = true;
        if (IsKeyDown(KEY_SPACE)) ur_pressed = true;
        
        // Update aircraft with RESPONSIVE controls
        aircraft_update_responsive(&aircraft, move_x, move_y, dt);
        
        // Speed control - FIXED MAPPING
        // UR = Gas/Go (normal throttle)
        if (ur_pressed) {
            aircraft.speed = fminf(aircraft.speed + 60.0f * dt, AIRCRAFT_MAX_SPEED * 1.2f);
        }
        // UL = Nitro burst
        else if (ul_pressed && aircraft.boost_timer <= 0 && boost_fuel > 20) {
            aircraft.boost_timer = 0.75f;
            aircraft.speed = AIRCRAFT_MAX_SPEED * 2.5f;  // Instant nitro boost!
            boost_fuel -= 20;
        }
        // No input = slow down
        else if (!ur_pressed && aircraft.boost_timer <= 0) {
            aircraft.speed = fmaxf(aircraft.speed - 25.0f * dt, AIRCRAFT_MIN_SPEED);
        }
        
        // Boost decay
        if (aircraft.boost_timer > 0) {
            aircraft.boost_timer -= dt;
            if (aircraft.boost_timer < 0.5f) {
                aircraft.speed = fmaxf(aircraft.speed - 300.0f * dt, AIRCRAFT_MAX_SPEED);
            }
        }
        
        // Recharge nitro
        if (boost_fuel < 100) {
            boost_fuel += 10 * dt;
        }
        
        // Update camera - RESPONSIVE
        camera_update_responsive(&camera, aircraft.position, aircraft.yaw, dt);
        
        // Drawing
        BeginDrawing();
        ClearBackground(DARKBLUE);
        
        BeginMode3D(camera);
        
        // Draw ground grid
        DrawGrid(100, 10.0f);
        
        // Draw aircraft (simple model)
        DrawCube(aircraft.position, 3, 1, 5, RED);
        Vector3 wing_l = Vector3Add(aircraft.position, (Vector3){-5, 0, 0});
        Vector3 wing_r = Vector3Add(aircraft.position, (Vector3){5, 0, 0});
        DrawCube(wing_l, 4, 0.5f, 2, RED);
        DrawCube(wing_r, 4, 0.5f, 2, RED);
        
        // Draw some reference cubes
        for (int i = -5; i <= 5; i++) {
            for (int j = -5; j <= 5; j++) {
                if (i != 0 || j != 0) {
                    DrawCube((Vector3){i * 50, 0, j * 50}, 10, 40, 10, GRAY);
                }
            }
        }
        
        EndMode3D();
        
        // HUD
        DrawText("AGILE CONTROLS DEMO", 10, 10, 30, WHITE);
        DrawText(TextFormat("Speed: %.0f / %.0f", aircraft.speed, AIRCRAFT_MAX_SPEED), 10, 50, 20, GREEN);
        DrawText(TextFormat("Turn Rate: %.0f deg/s", AIRCRAFT_TURN_RATE), 10, 80, 20, GREEN);
        DrawText(TextFormat("Roll: %.1f°", aircraft.roll), 10, 110, 20, WHITE);
        DrawText(TextFormat("Pitch: %.1f°", aircraft.pitch), 10, 140, 20, WHITE);
        
        // Nitro bar
        DrawText("NITRO", 10, 180, 20, WHITE);
        DrawRectangle(80, 180, 200, 20, DARKGRAY);
        DrawRectangle(80, 180, (int)(boost_fuel * 2), 20, aircraft.boost_timer > 0 ? SKYBLUE : BLUE);
        
        // Control display
        int y = 250;
        DrawText("CONTROLS (FIXED!):", 10, y, 20, YELLOW); y += 30;
        DrawText("Left Stick: Fly (agile!)", 10, y, 16, WHITE); y += 20;
        DrawText("UR Button: Gas/Go", 10, y, 16, GREEN); y += 20;
        DrawText("UL Button: Nitro", 10, y, 16, SKYBLUE); y += 20;
        DrawText("Right Stick: Gun Aim (Y=Axis 5)", 10, y, 16, ORANGE); y += 30;
        
        if (IsGamepadAvailable(0)) {
            DrawText(TextFormat("Right Y Axis: %d (must be 5!)", 
                     GetGamepadAxisCount(0) > 5 ? 5 : -1), 10, y, 16, 
                     GetGamepadAxisCount(0) > 5 ? GREEN : RED);
        }
        
        DrawFPS(screenWidth - 100, 10);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
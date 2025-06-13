/* Debug version of sky combat to find window issues */
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("=== SKY COMBAT DEBUG ===\n");
    
    // Set config flags for debugging
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN);
    
    printf("Initializing window...\n");
    InitWindow(800, 600, "Sky Combat Debug");
    
    if (!IsWindowReady()) {
        printf("ERROR: Window failed to initialize!\n");
        return 1;
    }
    
    printf("Window initialized successfully\n");
    printf("Window size: %dx%d\n", GetScreenWidth(), GetScreenHeight());
    
    SetTargetFPS(60);
    
    // Simple camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    printf("Starting main loop...\n");
    
    int frame = 0;
    while (!WindowShouldClose() && frame < 180) { // 3 seconds at 60 FPS
        frame++;
        
        if (frame % 60 == 0) {
            printf("Frame %d - Window is running\n", frame);
        }
        
        BeginDrawing();
        ClearBackground(DARKBLUE);
        
        BeginMode3D(camera);
        DrawGrid(10, 1.0f);
        DrawCube((Vector3){0.0f, 0.5f, 0.0f}, 2.0f, 1.0f, 2.0f, RED);
        EndMode3D();
        
        DrawText("Sky Combat Debug - Window is visible!", 10, 10, 20, WHITE);
        DrawText(TextFormat("Frame: %d", frame), 10, 40, 20, WHITE);
        DrawFPS(10, 70);
        
        EndDrawing();
    }
    
    printf("Closing window after %d frames\n", frame);
    CloseWindow();
    
    return 0;
}
/* Test program to debug raylib window issues */
#include <raylib.h>
#include <stdio.h>

int main() {
    printf("Starting raylib test...\n");
    
    // Initialize window with simple settings
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_UNDECORATED);
    InitWindow(800, 600, "Raylib Test Window");
    
    if (!IsWindowReady()) {
        printf("ERROR: Window failed to initialize!\n");
        return 1;
    }
    
    printf("Window initialized successfully\n");
    
    SetTargetFPS(60);
    
    // Don't disable cursor for testing
    // DisableCursor();
    
    int frame_count = 0;
    
    // Main game loop
    while (!WindowShouldClose() && frame_count < 300) {  // Exit after 5 seconds
        frame_count++;
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("Raylib Window Test", 200, 200, 20, BLACK);
        DrawText(TextFormat("Frame: %d", frame_count), 200, 250, 20, GRAY);
        DrawText("Press ESC to exit", 200, 300, 20, DARKGRAY);
        
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    printf("Closing window after %d frames\n", frame_count);
    CloseWindow();
    
    return 0;
}
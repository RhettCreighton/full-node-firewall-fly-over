#include <raylib.h>

int main() {
    InitWindow(800, 600, "Test Window");
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Press ESC to close", 10, 10, 20, BLACK);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
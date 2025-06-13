#!/bin/bash
# Build Sky Combat Ultimate with responsive controls fixed

echo "Building Sky Combat Ultimate with Responsive Controls..."

# Find all necessary source files
SOURCES="
src/controllers/sky_combat_ultimate.c
src/models/aircraft.c
src/models/aircraft_responsive.c
src/models/weapons.c
src/models/enemies.c
src/models/cyberpunk_world.c
src/models/overdrive.c
src/models/boss_cyber_dragon.c
src/views/combat_effects.c
src/views/camera_controller.c
src/controllers/input_mvc_fast.c
src/core/secure_code_points.c
src/core/specification.c
src/specifications/aircraft_controls_spec.c
"

# Add any missing dependencies
EXTRA_SOURCES=""

# Check for missing source files and add stubs if needed
for src in $SOURCES; do
    if [ ! -f "$src" ]; then
        echo "Warning: $src not found"
        # Create stub files for missing components
        case "$src" in
            *input_mvc_fast.c)
                echo "Creating stub for input_mvc_fast..."
                mkdir -p src/controllers
                cat > src/controllers/input_mvc_fast.c << 'EOF'
#include "sky_combat/controllers/input_mvc_fast.h"
#include <raylib.h>
#include <stdlib.h>

struct input_mvc_fast_t {
    int dummy;
};

input_mvc_fast_t* input_mvc_fast_create(void) {
    return calloc(1, sizeof(input_mvc_fast_t));
}

void input_mvc_fast_destroy(input_mvc_fast_t* input) {
    free(input);
}

input_view_fast_t input_mvc_fast_update(input_mvc_fast_t* input) {
    input_view_fast_t view = {0};
    return view;
}

input_state_fast_t input_view_get_state(input_view_fast_t view) {
    input_state_fast_t state = {0};
    
    // Get joystick input
    if (IsGamepadAvailable(0)) {
        state.move_x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        state.move_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        state.camera_x = GetGamepadAxisMovement(0, 4);  // Right stick X
        state.camera_y = GetGamepadAxisMovement(0, 5);  // Right stick Y (MUST BE 5!)
        
        state.fire_guns = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
        state.fire_missiles = IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
        state.speed_boost = IsGamepadButtonDown(0, 2);  // UL = boost
        state.brake = IsGamepadButtonDown(0, 3);        // UR = speed up
        state.barrel_roll_left = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        state.barrel_roll_right = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    }
    
    // Keyboard fallback
    if (IsKeyDown(KEY_W)) state.move_y = -1;
    if (IsKeyDown(KEY_S)) state.move_y = 1;
    if (IsKeyDown(KEY_A)) state.move_x = -1;
    if (IsKeyDown(KEY_D)) state.move_x = 1;
    
    if (IsKeyDown(KEY_SPACE)) state.fire_guns = true;
    if (IsKeyDown(KEY_LEFT_SHIFT)) state.speed_boost = true;
    if (IsKeyDown(KEY_LEFT_CONTROL)) state.brake = true;
    
    return state;
}
EOF
                ;;
        esac
    fi
done

# Create main wrapper if sky_combat_ultimate doesn't have main
cat > src/sky_combat_ultimate_main.c << 'EOF'
extern int sky_combat_ultimate_main(void);

int main(void) {
    return sky_combat_ultimate_main();
}
EOF

# Build the game
gcc -std=c99 \
    $SOURCES \
    $EXTRA_SOURCES \
    src/sky_combat_ultimate_main.c \
    -I./include \
    -lraylib -lm -lpthread \
    -o sky_combat_ultimate_fixed \
    -DAIRCRAFT_MAX_SPEED=120.0f \
    -DAIRCRAFT_MIN_SPEED=30.0f

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Run with: ./sky_combat_ultimate_fixed"
    chmod +x sky_combat_ultimate_fixed
else
    echo "Build failed!"
    exit 1
fi
#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

echo "=== CREATING CLEAN SKY COMBAT PROJECT ==="
echo

# Create the main CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(SkyCombat VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_SANITIZERS "Enable address and UB sanitizers" OFF)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# Global includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${RAYLIB_INCLUDE_DIRS})

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

if(ENABLE_SANITIZERS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

# Core library
add_library(sky_combat_core STATIC
    src/core/game_state.c
    src/core/command_system.c
    src/core/resource_manager.c
)

# Models library
add_library(sky_combat_models STATIC
    src/models/aircraft.c
    src/models/world.c
    src/models/character.c
    src/models/weapon.c
)

# Views library
add_library(sky_combat_views STATIC
    src/views/renderer.c
    src/views/hud.c
    src/views/camera.c
)

# Controllers library
add_library(sky_combat_controllers STATIC
    src/controllers/input_controller.c
    src/controllers/game_controller.c
)

# Systems library
add_library(sky_combat_systems STATIC
    src/systems/physics_system.c
    src/systems/ai_system.c
    src/systems/combat_system.c
)

# Main executable
add_executable(sky_combat
    src/main.c
)

target_link_libraries(sky_combat
    sky_combat_core
    sky_combat_models
    sky_combat_views
    sky_combat_controllers
    sky_combat_systems
    ${RAYLIB_LIBRARIES}
    m
    pthread
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS sky_combat DESTINATION bin)
install(DIRECTORY include/sky_combat DESTINATION include)

# Package configuration
include(CPack)
set(CPACK_PACKAGE_NAME "SkyCombat")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open world aircraft combat game")
EOF

# Create proper .gitignore
cat > .gitignore << 'EOF'
# Build directories
build/
cmake-build-*/
out/

# Archive directory
archive/

# Compiled binaries
*.o
*.a
*.so
*.dll
*.exe
sky_combat

# IDE files
.vscode/
.idea/
*.swp
*.swo
*~
.DS_Store

# Generated files
compile_commands.json
*.log

# Test outputs
tests/output/
*.gcov
*.gcda
*.gcno

# Documentation
docs/html/
docs/latex/
*.doxygen

# Temporary files
tmp/
temp/
EOF

# Create main.c with the working controls
cat > src/main.c << 'EOF'
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include "sky_combat/core/game_state.h"
#include "sky_combat/controllers/game_controller.h"
#include "sky_combat/views/renderer.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define TARGET_FPS 60

int main(int argc, char* argv[]) {
    // Initialize
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sky Combat - Open World");
    SetTargetFPS(TARGET_FPS);
    
    // Create game systems
    game_state_t* state = game_state_create();
    game_controller_t* controller = game_controller_create(state);
    renderer_t* renderer = renderer_create(state);
    
    if (!state || !controller || !renderer) {
        fprintf(stderr, "Failed to initialize game systems\n");
        return 1;
    }
    
    printf("Sky Combat - Open World Aircraft Combat\n");
    printf("Press F1 for help, ~ for console\n");
    
    // Main game loop
    while (!WindowShouldClose() && game_state_is_running(state)) {
        float dt = GetFrameTime();
        
        // Update
        game_controller_update(controller, dt);
        game_state_update(state, dt);
        
        // Render
        BeginDrawing();
        renderer_draw(renderer);
        EndDrawing();
    }
    
    // Cleanup
    renderer_destroy(renderer);
    game_controller_destroy(controller);
    game_state_destroy(state);
    CloseWindow();
    
    return 0;
}
EOF

# Create the aircraft model with working controls
cat > src/models/aircraft.c << 'EOF'
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/models/aircraft.h"
#include <stdlib.h>
#include <math.h>

// The proven control values
#define AIRCRAFT_BASE_SPEED 60.0f
#define AIRCRAFT_MAX_SPEED 120.0f  
#define AIRCRAFT_MIN_SPEED 30.0f
#define AIRCRAFT_TURN_RATE 150.0f
#define AIRCRAFT_PITCH_RATE 100.0f
#define AIRCRAFT_BOOST_MULTIPLIER 3.0f
#define AIRCRAFT_DRIFT_MULTIPLIER 2.5f

struct aircraft_s {
    Vector3 position;
    float yaw, pitch, roll;
    float speed;
    float actual_speed;
    bool boosting;
    bool drifting;
    float drift_factor;
    int health;
    int score;
};

aircraft_t* aircraft_create(Vector3 start_pos) {
    aircraft_t* aircraft = calloc(1, sizeof(aircraft_t));
    if (!aircraft) return NULL;
    
    aircraft->position = start_pos;
    aircraft->speed = AIRCRAFT_BASE_SPEED;
    aircraft->health = 100;
    
    return aircraft;
}

void aircraft_destroy(aircraft_t* aircraft) {
    free(aircraft);
}

void aircraft_update_controls(aircraft_t* aircraft,
                            float stick_x, float stick_y,
                            float aim_x, float aim_y,
                            bool boost, bool brake,
                            bool drift_left, bool drift_right,
                            float dt) {
    // Your proven control scheme
    aircraft->boosting = boost;
    
    // Drift mechanics
    float turn_rate = AIRCRAFT_TURN_RATE;
    aircraft->drift_factor = 1.0f;
    aircraft->drifting = false;
    
    if (stick_x < -0.1f && drift_right) {
        turn_rate *= AIRCRAFT_DRIFT_MULTIPLIER;
        aircraft->drift_factor = 0.7f;
        aircraft->drifting = true;
    } else if (stick_x > 0.1f && drift_left) {
        turn_rate *= AIRCRAFT_DRIFT_MULTIPLIER;
        aircraft->drift_factor = 0.7f;
        aircraft->drifting = true;
    }
    
    // Apply controls
    aircraft->yaw += stick_x * turn_rate * dt;
    aircraft->pitch = Lerp(aircraft->pitch, stick_y * 40.0f, 8.0f * dt);
    aircraft->roll = Lerp(aircraft->roll, -stick_x * (aircraft->drifting ? 60.0f : 35.0f), 5.0f * dt);
    
    // Speed
    if (brake) {
        aircraft->speed -= 60.0f * dt;
    } else if (!boost) {
        if (aircraft->pitch < 0) {
            aircraft->speed += (-aircraft->pitch / 30.0f) * 30.0f * dt;
        } else {
            aircraft->speed -= (aircraft->pitch / 30.0f) * 5.0f * dt;
        }
    }
    
    aircraft->speed = Clamp(aircraft->speed, AIRCRAFT_MIN_SPEED, AIRCRAFT_MAX_SPEED);
    aircraft->actual_speed = boost ? aircraft->speed * AIRCRAFT_BOOST_MULTIPLIER : aircraft->speed;
}
EOF

# Create README
cat > README.md << 'EOF'
# Sky Combat - Open World Aircraft Combat Game

A modular, open-world aircraft combat game with ground AI characters and enterable buildings.

## Features

- **Open World**: Large explorable world with dynamic loading
- **Aircraft Combat**: Fast-paced aerial combat with proven controls
- **Ground AI**: Autonomous characters that navigate and enter buildings
- **Enterable Buildings**: Detailed interiors with multiple floors
- **Command System**: Built-in console for testing and debugging
- **Modular Architecture**: Clean MVC design for easy extension

## Building

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Running

```bash
./build/sky_combat
```

## Controls

### Aircraft (ASTRO C40 / PlayStation layout)
- **Left Stick** (inverted): Fly aircraft
- **Right Stick**: Aim weapons (Y-axis on axis 5)
- **L2**: Fire missiles
- **R2**: Fire guns  
- **UR (Triangle)**: 3x Boost
- **UL (Square)**: Brake
- **L1/R1**: Drift turns

### Keyboard
- **WASD**: Fly aircraft
- **Arrow Keys**: Aim
- **Space**: Fire
- **Shift**: Boost
- **Z**: Brake
- **~**: Open console
- **F1**: Help

## Architecture

The project follows a clean MVC architecture:

- **Models**: Game objects (aircraft, world, characters)
- **Views**: Rendering and UI systems
- **Controllers**: Input handling and game logic
- **Core**: Central game state and systems
- **Systems**: Physics, AI, combat mechanics

## Development

### Adding Features

1. Create header in `include/sky_combat/feature/`
2. Implement in `src/feature/`
3. Add to appropriate CMakeLists.txt
4. Write tests in `tests/`

### Testing

```bash
cd build
ctest
```

## License

Apache 2.0 - See LICENSE file
EOF

echo "Main project structure created!"
echo
echo "Next steps:"
echo "1. Run: ./organize_project.sh  # To clean up old files"
echo "2. Run: mkdir build && cd build && cmake .. && make"
echo "3. Run: ./build/sky_combat"
echo
echo "The project is now professionally organized!"
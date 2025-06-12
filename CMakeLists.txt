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
    src/views/camera.c
)

# Controllers library
add_library(sky_combat_controllers STATIC
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
    # add_subdirectory(tests)  # TODO: Add when tests are created
endif()

# Installation
install(TARGETS sky_combat DESTINATION bin)
install(DIRECTORY include/sky_combat DESTINATION include)

# Package configuration
include(CPack)
set(CPACK_PACKAGE_NAME "SkyCombat")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open world aircraft combat game")

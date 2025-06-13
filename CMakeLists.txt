cmake_minimum_required(VERSION 3.10)
project(SkyCombat VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include philosophical proof system FIRST
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PhilosophicalProof.cmake)

# Include secure code points system
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SecureCodePoints.cmake)

# Enforce philosophical build barrier - cannot compile without proofs
philosophical_build_barrier()

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_SANITIZERS "Enable address and UB sanitizers" OFF)
option(REQUIRE_PHILOSOPHICAL_PROOFS "Require GDB proofs before compilation" ON)

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
    src/core/specification.c
    src/core/secure_code_points.c
)

# Models library
add_library(sky_combat_models STATIC
    src/models/aircraft.c
    src/models/aircraft_responsive.c
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

# Specifications library
add_library(sky_combat_specs STATIC
    src/specifications/aircraft_rendering_spec.c
    src/specifications/aircraft_controls_spec.c
    src/specifications/joystick_spec.c
    src/specifications/spec_init_order.c
    src/specifications/no_crash_spec.c
)

# Link specs library with core for specification system
target_link_libraries(sky_combat_specs
    sky_combat_core
)

# Main executable
add_executable(sky_combat
    src/main.c
)

# Specification-driven executable
add_executable(sky_combat_specs_demo
    src/main_with_specs.c
)

# Simple specification demo
add_executable(spec_demo
    src/simple_spec_demo.c
)

# Air drone test view
add_executable(air_drone_test
    src/views/air_drone_test_view.c
)

# Philosophical foundation test
add_executable(philosophical_test
    src/philosophical_test.c
)

# Secure joystick demo
add_executable(secure_joystick_demo
    src/secure_joystick_demo.c
    src/core/secure_code_points.c
)

# Apply philosophical proofs to all targets
if(REQUIRE_PHILOSOPHICAL_PROOFS)
    verify_all_specifications(sky_combat)
    verify_all_specifications(sky_combat_specs_demo)
    verify_all_specifications(air_drone_test)
    
    # Create specification to error code mappings
    create_spec_error_mapping("joystick_controls" 1001)
    create_spec_error_mapping("null_pointer_safety" 1002)
    create_spec_error_mapping("memory_bounds" 1003)
    create_spec_error_mapping("aircraft_physics" 1004)
    create_spec_error_mapping("input_latency" 1005)
    create_spec_error_mapping("rendering_pipeline" 1006)
endif()

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

target_link_libraries(sky_combat_specs_demo
    sky_combat_core
    sky_combat_models
    sky_combat_views
    sky_combat_controllers
    sky_combat_systems
    sky_combat_specs
    ${RAYLIB_LIBRARIES}
    m
    pthread
)

target_link_libraries(spec_demo
    sky_combat_core
    m
)

target_link_libraries(air_drone_test
    sky_combat_core
    sky_combat_models
    sky_combat_specs
    ${RAYLIB_LIBRARIES}
    m
)

target_link_libraries(philosophical_test
    m
)

target_link_libraries(secure_joystick_demo
    m
)

# 3D Cyberpunk Flying Game - The Fun Version!
add_executable(sky_combat_3d_simple
    sky_combat_3d_simple.c
)

target_link_libraries(sky_combat_3d_simple
    ${RAYLIB_LIBRARIES}
    m
)

# Joystick-Verified Flying Game with Secure Code Points
add_executable(sky_combat_joystick_verified
    sky_combat_joystick_verified.c
)

target_link_libraries(sky_combat_joystick_verified
    ${RAYLIB_LIBRARIES}
    m
    pthread
    secure_code_points
)

# Enable secure code points for the verified version
enable_secure_points(sky_combat_joystick_verified)

# Build joystick proofs if required
if(REQUIRE_PHILOSOPHICAL_PROOFS)
    build_joystick_proofs()
endif()

# Full Node: Firewall Fly-over - Enhanced gameplay version
add_executable(full-node-firewall-flyover
    src/controllers/full_node_main.c
    src/controllers/sky_combat_ultimate.c
    src/views/camera_controller.c
    src/controllers/input_mvc_fast.c
    src/controllers/input_controller_fast.c
    src/models/input_model_fast.c
    src/models/weapons.c
    src/models/enemies.c
    src/models/cyberpunk_world.c
    src/views/combat_effects.c
    src/models/overdrive.c
    src/models/boss_cyber_dragon.c
    src/utils/crash_protection.c
    modules/compile_time_gdb_proof/src/gdb_proof_sky_combat.c
    modules/compile_time_gdb_proof/src/gdb_proof_controls.c
    modules/compile_time_gdb_proof/src/gdb_proof_weapons.c
    modules/compile_time_gdb_proof/src/gdb_proof_gun_tuning.c
    modules/compile_time_gdb_proof/src/gdb_proof_practice_targets.c
)

# Add GDB proof include directory
target_include_directories(full-node-firewall-flyover PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof/include
)

target_link_libraries(full-node-firewall-flyover
    sky_combat_core
    sky_combat_models
    sky_combat_views
    sky_combat_controllers
    sky_combat_systems
    sky_combat_specs
    ${RAYLIB_LIBRARIES}
    m
    pthread
)

# Add GDB proof module dependency if enabled
if(TARGET gdb_proof_module)
    target_link_libraries(full-node-firewall-flyover gdb_proof_module)
endif()

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

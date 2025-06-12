cmake_minimum_required(VERSION 3.10)
project(sky_combat_modular C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# Main include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add all modules
add_subdirectory(modules/core)
add_subdirectory(modules/input)
add_subdirectory(modules/aircraft)
add_subdirectory(modules/world)
add_subdirectory(modules/characters)
add_subdirectory(modules/weapons)
add_subdirectory(modules/rendering)
add_subdirectory(modules/physics)
add_subdirectory(modules/commands)

# Main executable
add_executable(sky_combat_main 
    src/main.c
)

target_link_libraries(sky_combat_main
    sky_combat_core
    sky_combat_input
    sky_combat_aircraft
    sky_combat_world
    sky_combat_characters
    sky_combat_weapons
    sky_combat_rendering
    sky_combat_physics
    sky_combat_commands
    ${RAYLIB_LIBRARIES}
    m
)

# Test executables for individual features
add_executable(test_flight tests/test_flight.c)
target_link_libraries(test_flight 
    sky_combat_aircraft 
    sky_combat_input
    sky_combat_physics
    ${RAYLIB_LIBRARIES}
    m
)

add_executable(test_world tests/test_world.c)
target_link_libraries(test_world 
    sky_combat_world
    sky_combat_rendering
    ${RAYLIB_LIBRARIES}
    m
)

add_executable(test_characters tests/test_characters.c)
target_link_libraries(test_characters 
    sky_combat_characters
    sky_combat_world
    ${RAYLIB_LIBRARIES}
    m
)
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)
project(SecurePointsExample C)

# Include the secure code points system
include(${CMAKE_SOURCE_DIR}/../../cmake/SecureCodePoints.cmake)

# Extract all secure points from our source
set(ALL_POINTS
    WORLD_INIT_START
    WORLD_CHUNK_LOADED
    WORLD_SEAMLESS_READY
    LOADING_SCREEN_SHOWN
    PHYSICS_UPDATE_START
    CALCULATE_LIFT_FORCE
    APPLY_AERODYNAMICS
    STALL_CHECK
    STALL_WARNING
    VELOCITY_CHANGED
    POSITION_UPDATED
    PHYSICS_TELEPORT
    PHYSICS_INFINITE_ACCELERATION
    PHYSICS_IGNORE_GRAVITY
    AI_SPAWN_CHARACTER
    AI_ASSIGN_PERSONALITY
    AI_BEGIN_ROUTINE
    AI_PATH_PLANNING
    AI_AVOID_OBSTACLE
    AI_REACH_DESTINATION
    AI_REACT_TO_AIRCRAFT
    AI_WALK_THROUGH_BUILDING
    AI_TELEPORT_TO_DESTINATION
    AI_IDENTICAL_BEHAVIOR
)

# Build normal tracing binary
compile_proof_binary(simulator_trace
    ${CMAKE_SOURCE_DIR}/../../src/simulator/sky_combat_simulator.c
    TRACING
    ALL_PATHS ${ALL_POINTS}
)

# Build proof: World loads without loading screens
compile_proof_binary(proof_seamless_world
    ${CMAKE_SOURCE_DIR}/../../src/simulator/sky_combat_simulator.c
    PROVING
    ENABLE_PATHS
        WORLD_INIT_START
        WORLD_CHUNK_LOADED
        WORLD_SEAMLESS_READY
    DISABLE_PATHS
        LOADING_SCREEN_SHOWN
    ALL_PATHS ${ALL_POINTS}
)

# Build proof: Physics without teleporting
compile_proof_binary(proof_realistic_physics
    ${CMAKE_SOURCE_DIR}/../../src/simulator/sky_combat_simulator.c
    PROVING
    ENABLE_PATHS
        PHYSICS_UPDATE_START
        CALCULATE_LIFT_FORCE
        APPLY_AERODYNAMICS
        VELOCITY_CHANGED
        POSITION_UPDATED
        STALL_CHECK
        STALL_WARNING
    DISABLE_PATHS
        PHYSICS_TELEPORT
        PHYSICS_INFINITE_ACCELERATION
        PHYSICS_IGNORE_GRAVITY
    ALL_PATHS ${ALL_POINTS}
)

# Build proof: AI with realistic behavior
compile_proof_binary(proof_ai_behavior
    ${CMAKE_SOURCE_DIR}/../../src/simulator/sky_combat_simulator.c
    PROVING
    ENABLE_PATHS
        AI_SPAWN_CHARACTER
        AI_ASSIGN_PERSONALITY
        AI_BEGIN_ROUTINE
        AI_PATH_PLANNING
        AI_AVOID_OBSTACLE
        AI_REACH_DESTINATION
        AI_REACT_TO_AIRCRAFT
    DISABLE_PATHS
        AI_WALK_THROUGH_BUILDING
        AI_TELEPORT_TO_DESTINATION
        AI_IDENTICAL_BEHAVIOR
    ALL_PATHS ${ALL_POINTS}
)

# Run traced execution
add_traced_execution(trace_simulator simulator_trace simulator.trace)

# Add trace verification tests
enable_testing()
add_trace_test(verify_seamless_world simulator.trace SPEC "Seamless")
add_trace_test(verify_physics simulator.trace SPEC "Physics")
add_trace_test(verify_ai simulator.trace SPEC "AI")
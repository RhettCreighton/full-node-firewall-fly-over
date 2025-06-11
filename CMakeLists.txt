cmake_minimum_required(VERSION 3.10)
project(full-node-firewall-flyover C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add module paths
list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof"
    "${CMAKE_CURRENT_SOURCE_DIR}/robust_build_system/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

# Include safety systems
option(ENABLE_REAL_SAFETY "Enable real safety checks" ON)
if(ENABLE_REAL_SAFETY)
    include(SafetyChecks)
endif()

# Include compile-time GDB proof system (legacy)
option(ENABLE_GDB_PROOF "Enable compile-time GDB verification" OFF)
if(ENABLE_GDB_PROOF)
    add_subdirectory(modules/compile_time_gdb_proof)
    include(${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof/cmake/GDBProof.cmake OPTIONAL)
endif()

# Add REAL safety flags for compile-time checks
if(ENABLE_REAL_SAFETY)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wnull-dereference -Wstack-protector")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Warray-bounds=2 -Wfloat-equal")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wconversion -Wshadow -Wpointer-arith")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wcast-qual -Wstrict-overflow=5")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -ftrapv -D_FORTIFY_SOURCE=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-delete-null-pointer-checks")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -g")
endif()

# Find raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# Create library
add_library(sky_combat STATIC
    src/models/aircraft.c
    src/models/course.c
    src/models/weapons.c
    src/models/ai_pilot.c
    src/models/input_model.c
    src/models/enemies.c
    src/models/cyberpunk_world.c
    src/views/hud.c
    src/views/input_view.c
    src/views/combat_effects.c
    src/controllers/sky_combat.c
    src/controllers/input.c
    src/controllers/input_controller.c
    src/utils/crash_protection.c
    src/utils/specification_enforcement.c
)

target_include_directories(sky_combat PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${RAYLIB_INCLUDE_DIRS}
)

target_link_libraries(sky_combat PUBLIC
    ${RAYLIB_LIBRARIES}
    m
)

# THE ONE AND ONLY BINARY - FULL NODE: Firewall Fly-over
add_executable(full-node-firewall-flyover
    src/controllers/sky_combat_ultimate.c
    src/models/input_model_fast.c
    src/models/aircraft_responsive.c
    src/models/overdrive.c
    src/models/boss_cyber_dragon.c
    src/controllers/input_controller_fast.c
    src/controllers/input_mvc_fast.c
    src/controllers/input_astro_direct.c
    modules/compile_time_gdb_proof/src/gdb_proof_weapons.c
    modules/compile_time_gdb_proof/src/gdb_proof_gun_tuning.c
    modules/compile_time_gdb_proof/src/gdb_proof_practice_targets.c
)
target_include_directories(full-node-firewall-flyover PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof/include
)
if(ENABLE_GDB_PROOF)
    target_compile_definitions(full-node-firewall-flyover PRIVATE ENABLE_GDB_PROOF)
endif()
target_link_libraries(full-node-firewall-flyover sky_combat)
set_target_properties(full-node-firewall-flyover PROPERTIES
    OUTPUT_NAME "full-node-firewall-flyover"
)

# Apply REAL safety checks
if(ENABLE_REAL_SAFETY)
    # REQUIRE_SAFETY(full-node-firewall-flyover)
    
    # Add static analysis check
    add_custom_command(
        TARGET full-node-firewall-flyover
        PRE_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_no_coredump.py ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Running static analysis for crash prevention"
    )
endif()

# Apply legacy GDB verifications
if(ENABLE_GDB_PROOF)
    include(${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof/cmake/GDBProof.cmake)
    REQUIRE_LOCKED_CONTROLS(full-node-firewall-flyover)
    REQUIRE_WEAPON_INDEPENDENCE(full-node-firewall-flyover)
    REQUIRE_GUN_TUNING(full-node-firewall-flyover)
    REQUIRE_PRACTICE_TARGETS(full-node-firewall-flyover)
endif()

# Include new GDB proof system for no-coredump verification
option(ENABLE_NO_COREDUMP_PROOFS "Enable GDB no-coredump proofs" ON)
if(ENABLE_NO_COREDUMP_PROOFS AND CMAKE_BUILD_TYPE STREQUAL "Release")
    include(${CMAKE_CURRENT_SOURCE_DIR}/gdb_proof_system/integration/RunProofs.cmake)
    add_gdb_proofs(full-node-firewall-flyover)
    message(STATUS "GDB no-coredump proofs enabled for Release builds")
endif()

# MASTER VERIFICATION: Run ALL proofs at compile time
option(VERIFY_ALL_SPECIFICATIONS "Verify all specifications at compile time" ON)
if(VERIFY_ALL_SPECIFICATIONS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyAllProofs.cmake)
    verify_all_specifications(full-node-firewall-flyover)
endif()

# SPECIFICATION ENFORCEMENT: Make violations impossible
option(ENFORCE_SPECIFICATIONS "Enable specification enforcement system" ON)
if(ENFORCE_SPECIFICATIONS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SpecificationChecker.cmake)
    enforce_specifications(full-node-firewall-flyover)
    message(STATUS "Specification enforcement enabled - violations are IMPOSSIBLE")
endif()

# Install only the main binary
install(TARGETS full-node-firewall-flyover DESTINATION bin)
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Proof System CMake Module

# Function to add GDB safety verification to a Sky Combat executable
function(add_sky_combat_safety_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    # Get the module directory - we are in cmake/ subdirectory
    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add the proof source file to the target
    target_sources(${TARGET} PRIVATE 
        ${GDB_PROOF_MODULE_DIR}/src/gdb_proof_sky_combat.c
    )
    
    # Add include directory
    target_include_directories(${TARGET} PRIVATE
        ${GDB_PROOF_MODULE_DIR}/include
    )
    
    # Ensure debug symbols for GDB
    target_compile_options(${TARGET} PRIVATE -g)
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Safety Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_sky_combat_safety.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} has all safety guarantees"
        VERBATIM
    )
    
    message(STATUS "GDB safety verification enabled for ${TARGET}")
endfunction()

# Function to require a specific function is called
function(require_function_proof TARGET FUNCTION_NAME)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()
    
    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
    set(VERIFY_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/verify_${TARGET}_${FUNCTION_NAME}.sh")
    
    # Create custom verification script
    file(WRITE ${VERIFY_SCRIPT} "#!/bin/bash
BINARY=\"\$1\"
echo \"Verifying ${FUNCTION_NAME} is called...\"

gdb -batch -ex \"set pagination off\" \\
    -ex \"break ${FUNCTION_NAME}\" \\
    -ex \"run\" \\
    -ex \"if \\\$_isvoid(\\\$_breakpoint_hit)\" \\
    -ex \"  echo FAILED: ${FUNCTION_NAME} not called\" \\
    -ex \"  quit 1\" \\
    -ex \"else\" \\
    -ex \"  echo PASSED: ${FUNCTION_NAME} verified\" \\
    -ex \"  quit 0\" \\
    -ex \"end\" \\
    \"\$BINARY\" 2>&1 | grep -E \"(PASSED|FAILED)\"
")
    
    file(CHMOD ${VERIFY_SCRIPT} 
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
    
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${VERIFY_SCRIPT} $<TARGET_FILE:${TARGET}>
        COMMENT "Proof: ${TARGET} calls ${FUNCTION_NAME}"
        VERBATIM
    )
endfunction()

# Macro to make it easy to add all safety checks
macro(REQUIRE_NO_COREDUMP_GUARANTEE TARGET)
    add_sky_combat_safety_verification(${TARGET})
endmacro()

# Function to add control lock verification
function(add_control_lock_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add the control proof source
    target_sources(${TARGET} PRIVATE 
        ${GDB_PROOF_MODULE_DIR}/src/gdb_proof_controls.c
    )
    
    # Add include directory
    target_include_directories(${TARGET} PRIVATE
        ${GDB_PROOF_MODULE_DIR}/include
    )
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Control Lock Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_control_lock.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} has locked joystick controls"
        VERBATIM
    )
    
    message(STATUS "Control lock verification enabled for ${TARGET}")
endfunction()

# Macro to require both safety AND control lock
macro(REQUIRE_LOCKED_CONTROLS TARGET)
    add_sky_combat_safety_verification(${TARGET})
    add_control_lock_verification(${TARGET})
endmacro()

# Function to add view behavior verification
function(add_view_behavior_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add the view proof source
    target_sources(${TARGET} PRIVATE 
        ${GDB_PROOF_MODULE_DIR}/src/gdb_proof_view.c
    )
    
    # Add include directory
    target_include_directories(${TARGET} PRIVATE
        ${GDB_PROOF_MODULE_DIR}/include
    )
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB View Behavior Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_view_behavior.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} has correct view behavior"
        VERBATIM
    )
    
    message(STATUS "View behavior verification enabled for ${TARGET}")
endfunction()

# Macro for plane view demo
macro(REQUIRE_VIEW_BEHAVIOR TARGET)
    add_sky_combat_safety_verification(${TARGET})
    add_control_lock_verification(${TARGET})
    add_view_behavior_verification(${TARGET})
endmacro()

# Function to add weapon independence verification
function(add_weapon_independence_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Weapon Independence Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_weapon_independence.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} weapons don't affect movement"
        VERBATIM
    )
    
    message(STATUS "Weapon independence verification enabled for ${TARGET}")
endfunction()

# Macro to require weapon independence
macro(REQUIRE_WEAPON_INDEPENDENCE TARGET)
    add_weapon_independence_verification(${TARGET})
endmacro()

# Function to add right stick aim verification
function(add_right_stick_aim_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Right Stick Aim Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_right_stick_aim.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} uses right stick for aiming"
        VERBATIM
    )
    
    message(STATUS "Right stick aim verification enabled for ${TARGET}")
endfunction()

# Macro to require right stick aim control
macro(REQUIRE_RIGHT_STICK_AIM TARGET)
    add_right_stick_aim_verification(${TARGET})
endmacro()

# Function to add right stick unused verification
function(add_right_stick_unused_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Right Stick Unused Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_right_stick_unused.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} right stick has no function"
        VERBATIM
    )
    
    message(STATUS "Right stick unused verification enabled for ${TARGET}")
endfunction()

# Macro to require right stick unused
macro(REQUIRE_RIGHT_STICK_UNUSED TARGET)
    add_right_stick_unused_verification(${TARGET})
endmacro()

# Function to add straight shooting verification
function(add_straight_shooting_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Straight Shooting Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_straight_shooting.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} shoots bullets straight forward"
        VERBATIM
    )
    
    message(STATUS "Straight shooting verification enabled for ${TARGET}")
endfunction()

# Macro to require straight shooting
macro(REQUIRE_STRAIGHT_SHOOTING TARGET)
    add_straight_shooting_verification(${TARGET})
endmacro()

# Function to add gun tuning verification
function(add_gun_tuning_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Gun Tuning Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_gun_tuning.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} has gun tuning system (right stick fine-tunes, bullets go straight)"
        VERBATIM
    )
    
    message(STATUS "Gun tuning verification enabled for ${TARGET}")
endfunction()

# Macro to require gun tuning system
macro(REQUIRE_GUN_TUNING TARGET)
    add_gun_tuning_verification(${TARGET})
endmacro()

# Function to add practice targets verification
function(add_practice_targets_verification TARGET)
    if(NOT ENABLE_GDB_PROOF)
        return()
    endif()

    set(GDB_PROOF_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/compile_time_gdb_proof")
    
    # Add post-build verification
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Practice Targets Verification for ${TARGET} ==="
        COMMAND ${GDB_PROOF_MODULE_DIR}/scripts/verify_practice_targets.sh $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} has practice targets with controlled explosions"
        VERBATIM
    )
    
    message(STATUS "Practice targets verification enabled for ${TARGET}")
endfunction()

# Macro to require practice targets
macro(REQUIRE_PRACTICE_TARGETS TARGET)
    add_practice_targets_verification(${TARGET})
endmacro()
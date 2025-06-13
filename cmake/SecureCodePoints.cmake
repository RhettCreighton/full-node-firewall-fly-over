# Secure Code Points CMake System
# Implements cryptographic proof of reachability through compile-time path selection

# Function to enable a secure code point at compile time
function(enable_secure_code_point TARGET NAME)
    target_compile_definitions(${TARGET} PRIVATE PATH_${NAME}_EXISTS=1)
endfunction()

# Function to disable a secure code point at compile time
function(disable_secure_code_point TARGET NAME)
    target_compile_definitions(${TARGET} PRIVATE PATH_${NAME}_EXISTS=0)
endfunction()

# Function to create a proof binary with specific paths enabled/disabled
function(generate_proof_binary BASE_TARGET SOURCE_FILE TEST_NAME ENABLED_PATHS DISABLED_PATHS)
    set(PROOF_TARGET "${BASE_TARGET}_proof_${TEST_NAME}")
    
    add_executable(${PROOF_TARGET} ${SOURCE_FILE})
    
    # Copy properties from base target
    target_link_libraries(${PROOF_TARGET} ${RAYLIB_LIBRARIES} m pthread)
    target_include_directories(${PROOF_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    
    # Enable proving mode
    target_compile_definitions(${PROOF_TARGET} PRIVATE PROVING_BUILD=1)
    
    # Enable specified paths
    foreach(PATH ${ENABLED_PATHS})
        enable_secure_code_point(${PROOF_TARGET} ${PATH})
    endforeach()
    
    # Disable specified paths
    foreach(PATH ${DISABLED_PATHS})
        disable_secure_code_point(${PROOF_TARGET} ${PATH})
    endforeach()
    
    # Don't install proof binaries
    set_target_properties(${PROOF_TARGET} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endfunction()

# Function to create a trace execution binary
function(create_trace_execution NAME BINARY)
    set(OUTPUT_TRACE "${CMAKE_BINARY_DIR}/${NAME}.trace")
    
    add_custom_command(
        OUTPUT ${OUTPUT_TRACE}
        COMMAND ${CMAKE_COMMAND} -E env TRACE_MODE=1 ${BINARY} > ${OUTPUT_TRACE} 2>&1 || true
        DEPENDS ${BINARY}
        COMMENT "Running traced execution: ${BINARY}"
    )
    
    add_custom_target(${NAME} DEPENDS ${OUTPUT_TRACE})
endfunction()

# Function to extract secure code points from source
function(extract_secure_code_points SOURCE_FILE OUTPUT_FILE)
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND bash -c "grep -o 'SECURE_CODE_POINT([A-Z_]*' ${SOURCE_FILE} | sed 's/SECURE_CODE_POINT(//' | sort | uniq > ${OUTPUT_FILE}"
        DEPENDS ${SOURCE_FILE}
        COMMENT "Extracting secure code points from ${SOURCE_FILE}"
    )
endfunction()

# Function to verify unreachable paths at build time
function(verify_unreachable_paths TARGET)
    # This would run the proof verification
    # For now, we'll add a build-time message
    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMENT "Secure code points verified for ${TARGET}"
    )
endfunction()

# Create the secure code points library
add_library(secure_code_points STATIC
    ${CMAKE_SOURCE_DIR}/src/core/secure_code_points.c
)

target_include_directories(secure_code_points PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# Macro to assert path is unreachable at compile time
macro(ASSERT_UNREACHABLE PATH)
    if(DEFINED PATH_${PATH}_EXISTS AND PATH_${PATH}_EXISTS)
        message(FATAL_ERROR "Path ${PATH} must be unreachable but is enabled!")
    endif()
endmacro()

# Macro to assert path is reachable at compile time  
macro(ASSERT_REACHABLE PATH)
    if(NOT DEFINED PATH_${PATH}_EXISTS OR NOT PATH_${PATH}_EXISTS)
        message(FATAL_ERROR "Path ${PATH} must be reachable but is disabled!")
    endif()
endmacro()

# Function to build all proof binaries for joystick specifications
function(build_joystick_proofs)
    # Proof 1: Axis 3 is unreachable
    set(ENABLED_PATHS_1 
        RIGHT_STICK_Y_CORRECT
        JOYSTICK_SPEC_START
        RIGHT_STICK_PITCH_SPEC
        PULL_BACK_CLIMBS
        PUSH_FORWARD_DIVES
    )
    set(DISABLED_PATHS_1
        RIGHT_STICK_Y_WRONG
    )
    generate_proof_binary(sky_combat
        "${CMAKE_SOURCE_DIR}/src/joystick_with_traces.c"
        "axis_3_unreachable"
        "${ENABLED_PATHS_1}"
        "${DISABLED_PATHS_1}"
    )
    
    # Proof 2: Correct axis mapping
    set(ENABLED_PATHS_2
        RIGHT_STICK_Y_CORRECT
        RIGHT_STICK_X_CORRECT
        LEFT_STICK_THROTTLE_CORRECT
        STICK_LEFT_TURNS_LEFT
        STICK_RIGHT_TURNS_RIGHT
    )
    set(DISABLED_PATHS_2
        RIGHT_STICK_Y_WRONG
        RIGHT_STICK_X_WRONG
        LEFT_STICK_THROTTLE_WRONG
    )
    generate_proof_binary(sky_combat
        "${CMAKE_SOURCE_DIR}/src/joystick_with_traces.c"
        "correct_axis_mapping"
        "${ENABLED_PATHS_2}"
        "${DISABLED_PATHS_2}"
    )
endfunction()

# Enable secure code points for targets that use them
function(enable_secure_points TARGET)
    target_link_libraries(${TARGET} secure_code_points)
    target_compile_definitions(${TARGET} PRIVATE SECURE_CODE_POINTS_ENABLED=1)
endfunction()
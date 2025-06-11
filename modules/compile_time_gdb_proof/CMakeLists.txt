# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Compile-Time GDB Proof System Module

# This module provides compile-time verification using GDB
# It can verify that certain functions are called and behaviors occur

# Define the module
add_library(compile_time_gdb_proof INTERFACE)

# Set up include directories
target_include_directories(compile_time_gdb_proof INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Install scripts for use by other targets
install(FILES
    scripts/verify_final.sh
    scripts/verify_pixel_draw.sh
    scripts/verify_pixel_draw_v2.sh
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/compile_time_gdb_proof/scripts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# Example/test executables
if(BUILD_GDB_PROOF_EXAMPLES)
    # Firefox pixel example
    add_executable(firefox_app src/firefox_app.c)
    target_compile_options(firefox_app PRIVATE -g -O0)
    
    # Bad example (should fail verification)
    add_executable(bad_app src/bad_example.c)
    target_compile_options(bad_app PRIVATE -g -O0)
    
    # Tricky example (has function but doesn't call it)
    add_executable(tricky_app src/tricky_example.c)
    target_compile_options(tricky_app PRIVATE -g -O0)
endif()

# Custom function to add GDB verification to a target
function(add_gdb_verification TARGET VERIFICATION_SCRIPT)
    # Get the source directory for scripts
    set(SCRIPT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
    
    # Add custom command that runs before building the target
    add_custom_command(
        TARGET ${TARGET}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Verification for ${TARGET} ==="
        COMMAND ${CMAKE_COMMAND} -E echo "Running ${VERIFICATION_SCRIPT}..."
        COMMAND chmod +x ${SCRIPT_DIR}/${VERIFICATION_SCRIPT}
        COMMAND ${SCRIPT_DIR}/${VERIFICATION_SCRIPT} $<TARGET_FILE:${TARGET}>
        COMMENT "Performing GDB compile-time verification for ${TARGET}"
        VERBATIM
    )
endfunction()

# Function to require a specific function call in a target
function(require_function_call TARGET FUNCTION_NAME)
    set(SCRIPT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
    
    # Create a custom verification script for this specific function
    set(VERIFY_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/verify_${TARGET}_${FUNCTION_NAME}.sh")
    
    file(WRITE ${VERIFY_SCRIPT} "#!/bin/bash
# Auto-generated verification script
BINARY=\"\$1\"
if [ ! -f \"\$BINARY\" ]; then
    echo \"ERROR: Binary \$BINARY not found\"
    exit 1
fi

# Create GDB script to check for function
cat > /tmp/verify_${FUNCTION_NAME}.gdb << 'EOF'
set pagination off
set confirm off
break ${FUNCTION_NAME}
run
if \$_isvoid(\$_breakpoint_hit)
  echo VERIFICATION FAILED: Function ${FUNCTION_NAME} not called\\n
  quit 1
else
  echo VERIFICATION PASSED: Function ${FUNCTION_NAME} is called\\n
  quit 0
end
EOF

# Run GDB verification
gdb -batch -x /tmp/verify_${FUNCTION_NAME}.gdb \"\$BINARY\" 2>&1
RESULT=\$?
rm -f /tmp/verify_${FUNCTION_NAME}.gdb
exit \$RESULT
")
    
    file(CHMOD ${VERIFY_SCRIPT} 
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                    GROUP_READ GROUP_EXECUTE 
                    WORLD_READ WORLD_EXECUTE)
    
    # Add as a POST_BUILD command
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${VERIFY_SCRIPT} $<TARGET_FILE:${TARGET}>
        COMMENT "Verifying ${TARGET} calls ${FUNCTION_NAME}"
        VERBATIM
    )
endfunction()

# Export functions for use in parent scope
set(COMPILE_TIME_GDB_PROOF_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
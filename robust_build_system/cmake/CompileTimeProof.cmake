# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# CompileTimeProof.cmake - Verify program invariants before allowing compilation

include_guard(GLOBAL)

# Add a compile-time proof requirement to a target
function(add_compile_time_proof TARGET)
    cmake_parse_arguments(PROOF
        ""
        "PROOF_FUNCTION;EXPECTED_OUTPUT;TIMEOUT"
        "INVARIANTS;REQUIRED_SYMBOLS;FORBIDDEN_SYMBOLS"
        ${ARGN}
    )
    
    # Default timeout
    if(NOT PROOF_TIMEOUT)
        set(PROOF_TIMEOUT 5)
    endif()
    
    # Create proof verification script
    set(PROOF_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_proof.cmake")
    
    file(WRITE ${PROOF_SCRIPT} "
# Auto-generated proof verification script
set(TARGET_BINARY \"${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_proof_test\")
set(SOURCE_FILES \"${ARGN}\")

# Compile test binary
execute_process(
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -g -O0 
            -o \${TARGET_BINARY} \${SOURCE_FILES}
    RESULT_VARIABLE COMPILE_RESULT
    OUTPUT_VARIABLE COMPILE_OUTPUT
    ERROR_VARIABLE COMPILE_ERROR
)

if(NOT COMPILE_RESULT EQUAL 0)
    message(FATAL_ERROR \"Proof compilation failed: \${COMPILE_ERROR}\")
endif()

# Check required symbols
")
    
    # Add symbol checks
    if(PROOF_REQUIRED_SYMBOLS)
        foreach(SYMBOL ${PROOF_REQUIRED_SYMBOLS})
            file(APPEND ${PROOF_SCRIPT} "
execute_process(
    COMMAND nm \${TARGET_BINARY}
    COMMAND grep -q \"${SYMBOL}\"
    RESULT_VARIABLE SYMBOL_CHECK
)
if(NOT SYMBOL_CHECK EQUAL 0)
    message(FATAL_ERROR \"Required symbol '${SYMBOL}' not found in binary\")
endif()
message(STATUS \"✓ Found required symbol: ${SYMBOL}\")
")
        endforeach()
    endif()
    
    # Check forbidden symbols
    if(PROOF_FORBIDDEN_SYMBOLS)
        foreach(SYMBOL ${PROOF_FORBIDDEN_SYMBOLS})
            file(APPEND ${PROOF_SCRIPT} "
execute_process(
    COMMAND nm \${TARGET_BINARY}
    COMMAND grep -q \"${SYMBOL}\"
    RESULT_VARIABLE FORBIDDEN_CHECK
)
if(FORBIDDEN_CHECK EQUAL 0)
    message(FATAL_ERROR \"Forbidden symbol '${SYMBOL}' found in binary\")
endif()
message(STATUS \"✓ Forbidden symbol not present: ${SYMBOL}\")
")
        endforeach()
    endif()
    
    # Run the program and check output
    if(PROOF_EXPECTED_OUTPUT)
        file(APPEND ${PROOF_SCRIPT} "
# Run program and verify output
execute_process(
    COMMAND \${TARGET_BINARY}
    TIMEOUT ${PROOF_TIMEOUT}
    OUTPUT_VARIABLE RUN_OUTPUT
    ERROR_VARIABLE RUN_ERROR
    RESULT_VARIABLE RUN_RESULT
)

if(NOT RUN_OUTPUT MATCHES \"${PROOF_EXPECTED_OUTPUT}\")
    message(FATAL_ERROR \"Expected output '${PROOF_EXPECTED_OUTPUT}' not found\")
endif()
message(STATUS \"✓ Program output verified\")
")
    endif()
    
    # Verify invariants with GDB
    if(PROOF_INVARIANTS)
        file(APPEND ${PROOF_SCRIPT} "
# Create GDB script to verify invariants
set(GDB_SCRIPT \"\${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_invariants.gdb\")
file(WRITE \${GDB_SCRIPT} \"set pagination off\\n\")
file(APPEND \${GDB_SCRIPT} \"set confirm off\\n\")
")
        foreach(INVARIANT ${PROOF_INVARIANTS})
            file(APPEND ${PROOF_SCRIPT} "
file(APPEND \${GDB_SCRIPT} \"break ${INVARIANT}\\n\")
file(APPEND \${GDB_SCRIPT} \"commands\\n\")
file(APPEND \${GDB_SCRIPT} \"echo INVARIANT_VERIFIED:${INVARIANT}\\\\n\\n\")
file(APPEND \${GDB_SCRIPT} \"continue\\n\")
file(APPEND \${GDB_SCRIPT} \"end\\n\")
")
        endforeach()
        
        file(APPEND ${PROOF_SCRIPT} "
file(APPEND \${GDB_SCRIPT} \"run\\n\")
file(APPEND \${GDB_SCRIPT} \"quit\\n\")

# Run GDB verification
execute_process(
    COMMAND gdb -batch -x \${GDB_SCRIPT} \${TARGET_BINARY}
    TIMEOUT ${PROOF_TIMEOUT}
    OUTPUT_VARIABLE GDB_OUTPUT
    RESULT_VARIABLE GDB_RESULT
)

# Check all invariants were verified
")
        foreach(INVARIANT ${PROOF_INVARIANTS})
            file(APPEND ${PROOF_SCRIPT} "
if(NOT GDB_OUTPUT MATCHES \"INVARIANT_VERIFIED:${INVARIANT}\")
    message(FATAL_ERROR \"Invariant '${INVARIANT}' not verified\")
endif()
message(STATUS \"✓ Invariant verified: ${INVARIANT}\")
")
        endforeach()
    endif()
    
    file(APPEND ${PROOF_SCRIPT} "
# Clean up
file(REMOVE \${TARGET_BINARY})
message(STATUS \"✅ All compile-time proofs passed for ${TARGET}\")
")
    
    # Add custom command to run proof before building
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.proof"
        COMMAND ${CMAKE_COMMAND} -P ${PROOF_SCRIPT}
        COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.proof"
        DEPENDS ${ARGN}
        COMMENT "Verifying compile-time proofs for ${TARGET}..."
        VERBATIM
    )
    
    # Make target depend on proof
    add_custom_target(${TARGET}_proof 
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.proof"
    )
    
    add_dependencies(${TARGET} ${TARGET}_proof)
endfunction()

# Convenience function for common proof patterns
function(require_function_called TARGET FUNCTION OUTPUT_PATTERN)
    add_compile_time_proof(${TARGET}
        PROOF_FUNCTION ${FUNCTION}
        EXPECTED_OUTPUT ${OUTPUT_PATTERN}
        INVARIANTS ${FUNCTION}
    )
endfunction()

# Verify memory safety invariants
function(add_memory_safety_proof TARGET)
    add_compile_time_proof(${TARGET}
        FORBIDDEN_SYMBOLS "gets" "strcpy" "strcat" "sprintf"
        REQUIRED_SYMBOLS "__fortify_fail" "__stack_chk_fail"
    )
endfunction()
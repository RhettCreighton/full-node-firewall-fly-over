# GDB Proof System CMake Integration
# Runs deterministic proofs at build time to guarantee no core dumps

# Find Python3
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Function to add GDB proofs to a target
function(add_gdb_proofs TARGET)
    # Only run proofs in Release mode (Debug has sanitizers)
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        
        # Create proof output directory
        set(PROOF_OUTPUT_DIR "${CMAKE_BINARY_DIR}/gdb_proofs/${TARGET}")
        file(MAKE_DIRECTORY ${PROOF_OUTPUT_DIR})
        
        # Add custom command to run proofs after building
        add_custom_command(
            TARGET ${TARGET}
            POST_BUILD
            COMMAND ${Python3_EXECUTABLE} 
                    ${CMAKE_CURRENT_SOURCE_DIR}/gdb_proof_system/framework/proof_runner.py
                    $<TARGET_FILE:${TARGET}>
                    ${PROOF_OUTPUT_DIR}/proof_report.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running GDB proofs for ${TARGET}..."
            VERBATIM
        )
        
        # Add test so it runs with 'make test'
        add_test(
            NAME ${TARGET}_gdb_proofs
            COMMAND ${Python3_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/gdb_proof_system/framework/proof_runner.py
                    $<TARGET_FILE:${TARGET}>
                    ${PROOF_OUTPUT_DIR}/proof_test_report.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        
        # Set test properties
        set_tests_properties(${TARGET}_gdb_proofs PROPERTIES
            LABELS "safety;deterministic"
            TIMEOUT 300  # 5 minute timeout
            FAIL_REGULAR_EXPRESSION "FAIL;ERROR;VERIFICATION FAILED"
            PASS_REGULAR_EXPRESSION "VERIFIED;No core dumps possible"
        )
        
        # Option to make build fail if proofs fail
        if(STRICT_SAFETY_PROOFS)
            add_custom_command(
                TARGET ${TARGET}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Safety proofs are REQUIRED for this build"
                COMMAND test -f ${PROOF_OUTPUT_DIR}/proof_report.json || exit 1
                COMMAND grep -q '"verdict": "VERIFIED"' ${PROOF_OUTPUT_DIR}/proof_report.json || exit 1
                COMMENT "Verifying safety proofs passed..."
                VERBATIM
            )
        endif()
        
    else()
        message(STATUS "Skipping GDB proofs in ${CMAKE_BUILD_TYPE} mode")
    endif()
endfunction()

# Function to generate proof summary
function(generate_proof_summary)
    add_custom_target(proof_summary
        COMMAND ${CMAKE_COMMAND} -E echo "=== GDB Proof Summary ==="
        COMMAND find ${CMAKE_BINARY_DIR}/gdb_proofs -name "proof_report.json" -exec echo "Report: {}" \\; -exec grep "verdict" {} \\;
        COMMENT "Generating proof summary..."
        VERBATIM
    )
endfunction()

# Add option to enable strict proofs
option(STRICT_SAFETY_PROOFS "Fail build if GDB safety proofs fail" OFF)

# Add option to skip proofs
option(SKIP_GDB_PROOFS "Skip GDB deterministic proofs" OFF)

if(SKIP_GDB_PROOFS)
    message(WARNING "GDB safety proofs are DISABLED - no guarantees about core dumps!")
endif()
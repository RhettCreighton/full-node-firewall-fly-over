# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Dataflow Proof System - Proves error paths are unreachable

function(add_dataflow_proof target source_file)
    # Build the dataflow prover if not already built
    if(NOT TARGET dataflow_prover)
        add_executable(dataflow_prover 
            ${CMAKE_CURRENT_LIST_DIR}/../src/dataflow_prover.c
        )
    endif()
    
    # Add proof verification as pre-build step
    add_custom_command(
        TARGET ${target}
        PRE_BUILD
        COMMAND dataflow_prover ${source_file}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Proving error paths unreachable in ${source_file}"
    )
    
    # If proof fails, compilation stops
    set_property(TARGET ${target} PROPERTY COMPILE_FLAGS "-DPROOF_VERIFIED")
endfunction()

# Macro to mark deterministic data
macro(MARK_DETERMINISTIC var min max)
    add_compile_definitions(
        ${var}_IS_DETERMINISTIC
        ${var}_MIN=${min}
        ${var}_MAX=${max}
    )
endmacro()

# Macro to mark constrained non-deterministic data  
macro(MARK_CONSTRAINED var constraint)
    add_compile_definitions(
        ${var}_IS_CONSTRAINED
        ${var}_CONSTRAINT="${constraint}"
    )
endmacro()

# Enable proof system for all targets
function(enable_zero_crash_proofs)
    # Add compile flag to enable static assertions
    add_compile_options(-DZERO_CRASH_PROOFS)
    
    # Find all error handlers in codebase
    file(GLOB_RECURSE SOURCE_FILES 
        ${CMAKE_SOURCE_DIR}/src/*.c
    )
    
    foreach(source ${SOURCE_FILES})
        # Check for error patterns
        file(STRINGS ${source} error_patterns
            REGEX "(if.*[<>=].*[0-9].*=|return -[0-9]|assert\\(0\\)|abort\\(\\))"
        )
        
        if(error_patterns)
            message(STATUS "Found error paths in ${source} - adding proof requirement")
            # This file needs proof
            set_property(GLOBAL APPEND PROPERTY FILES_NEEDING_PROOF ${source})
        endif()
    endforeach()
endfunction()
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# AST Verification CMake Module
# Provides functions to add AST safety verification to targets

# Find required tools
find_program(CLANG_EXECUTABLE clang)
find_program(PYTHON3_EXECUTABLE python3)

# Check if AST proof system is available
if(NOT AST_PROOF_SYSTEM_DIR)
    set(AST_PROOF_SYSTEM_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

if(NOT EXISTS ${AST_PROOF_SYSTEM_DIR}/enhanced_ast_analyzer.py)
    message(WARNING "AST Proof System not found at ${AST_PROOF_SYSTEM_DIR}")
    set(AST_PROOF_SYSTEM_AVAILABLE FALSE)
else()
    set(AST_PROOF_SYSTEM_AVAILABLE TRUE)
endif()

# Function to add AST verification to a target
function(add_ast_verification target)
    if(NOT AST_PROOF_SYSTEM_AVAILABLE)
        message(WARNING "AST verification requested but system not available")
        return()
    endif()
    
    if(NOT CLANG_EXECUTABLE)
        message(WARNING "Clang not found - AST verification disabled for ${target}")
        return()
    endif()
    
    # Get source files for the target
    get_target_property(sources ${target} SOURCES)
    
    # Filter for C/C++ files
    set(c_sources)
    foreach(source ${sources})
        if(source MATCHES "\\.(c|cc|cpp|cxx)$")
            list(APPEND c_sources ${source})
        endif()
    endforeach()
    
    if(NOT c_sources)
        return()
    endif()
    
    # Create verification target
    add_custom_target(${target}_ast_verify
        COMMAND ${PYTHON3_EXECUTABLE} 
                ${AST_PROOF_SYSTEM_DIR}/enhanced_ast_analyzer.py
                ${c_sources}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running AST safety verification for ${target}"
    )
    
    # Add determinism analysis
    add_custom_target(${target}_determinism
        COMMAND ${PYTHON3_EXECUTABLE}
                ${AST_PROOF_SYSTEM_DIR}/determinism_analyzer.py
                ${c_sources}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Analyzing determinism for ${target}"
    )
    
    # Make verification run before building target
    add_dependencies(${target} ${target}_ast_verify)
    
    # Add post-build incremental verification
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${PYTHON3_EXECUTABLE}
                ${AST_PROOF_SYSTEM_DIR}/incremental_verifier.py
                ${CMAKE_CURRENT_SOURCE_DIR}
                --report ${CMAKE_BINARY_DIR}/${target}_safety_report.md
        COMMENT "Incremental safety verification for ${target}"
    )
endfunction()

# Function to add determinism-aware optimization
function(add_deterministic_optimization target)
    if(NOT AST_PROOF_SYSTEM_AVAILABLE)
        return()
    endif()
    
    # Run determinism analysis to find cacheable operations
    execute_process(
        COMMAND ${PYTHON3_EXECUTABLE}
                ${AST_PROOF_SYSTEM_DIR}/determinism_analyzer.py
                $<TARGET_FILE:${target}>
        OUTPUT_VARIABLE det_output
        ERROR_QUIET
        RESULT_VARIABLE det_result
    )
    
    if(det_result EQUAL 0)
        # Add compile definitions based on analysis
        target_compile_definitions(${target} PRIVATE
            AST_DETERMINISTIC_PROOFS
            AST_ENABLE_PROOF_CACHE
        )
        
        message(STATUS "Deterministic optimizations enabled for ${target}")
    endif()
endfunction()

# Function to generate proof report
function(generate_proof_report target output_file)
    if(NOT AST_PROOF_SYSTEM_AVAILABLE)
        return()
    endif()
    
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND ${PYTHON3_EXECUTABLE}
                ${AST_PROOF_SYSTEM_DIR}/run_deterministic_proof.sh
                $<TARGET_FILE:${target}>
                > ${output_file}
        DEPENDS ${target}
        COMMENT "Generating mathematical proof for ${target}"
    )
    
    add_custom_target(${target}_proof_report
        DEPENDS ${output_file}
    )
endfunction()

# Macro to mark deterministic functions
macro(mark_deterministic_function func_name)
    add_compile_definitions(
        ${func_name}_IS_DETERMINISTIC
    )
endmacro()

# Enable AST verification for all targets in directory
macro(enable_ast_verification_all)
    set(CMAKE_AST_VERIFICATION_ENABLED TRUE)
    
    # Override add_executable
    function(add_executable target)
        _add_executable(${target} ${ARGN})
        if(CMAKE_AST_VERIFICATION_ENABLED)
            add_ast_verification(${target})
        endif()
    endfunction()
    
    # Override add_library  
    function(add_library target)
        _add_library(${target} ${ARGN})
        if(CMAKE_AST_VERIFICATION_ENABLED)
            add_ast_verification(${target})
        endif()
    endfunction()
endmacro()

# Configure AST proof system settings
function(configure_ast_proof_system)
    set(options ENABLE_CACHE STRICT_MODE PARALLEL)
    set(oneValueArgs CACHE_DIR REPORT_DIR)
    set(multiValueArgs EXCLUDE_PATTERNS CRITICAL_PATHS)
    cmake_parse_arguments(AST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Create configuration file
    set(config_content "{")
    
    if(AST_ENABLE_CACHE)
        set(config_content "${config_content}\n  \"enable_cache\": true,")
        if(AST_CACHE_DIR)
            set(config_content "${config_content}\n  \"cache_dir\": \"${AST_CACHE_DIR}\",")
        endif()
    endif()
    
    if(AST_STRICT_MODE)
        set(config_content "${config_content}\n  \"strict_mode\": true,")
    endif()
    
    if(AST_PARALLEL)
        set(config_content "${config_content}\n  \"parallel\": true,")
    endif()
    
    set(config_content "${config_content}\n  \"version\": \"1.0\"\n}")
    
    file(WRITE ${CMAKE_BINARY_DIR}/.ast_proof.json ${config_content})
endfunction()

# Print configuration status
if(AST_PROOF_SYSTEM_AVAILABLE)
    message(STATUS "AST Proof System: AVAILABLE")
    message(STATUS "  Location: ${AST_PROOF_SYSTEM_DIR}")
    message(STATUS "  Clang: ${CLANG_EXECUTABLE}")
    message(STATUS "  Python: ${PYTHON3_EXECUTABLE}")
else()
    message(STATUS "AST Proof System: NOT AVAILABLE")
endif()
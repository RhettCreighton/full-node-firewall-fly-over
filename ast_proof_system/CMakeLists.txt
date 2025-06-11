cmake_minimum_required(VERSION 3.10)
project(ASTProofSystem VERSION 1.0.0 LANGUAGES C)

# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Strict C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -O2")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required tools
find_program(CLANG_EXECUTABLE clang REQUIRED)
find_program(GDB_EXECUTABLE gdb REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(AST_PROOF_SOURCES
    src/ast_analyzer.c
    src/determinism_analyzer.c
    src/constraint_verifier.c
    src/main.c
)

# Main executable
add_executable(firewall-fly-over ${AST_PROOF_SOURCES})
target_link_libraries(firewall-fly-over m)

# Test programs
add_executable(test_dangerous test_dangerous_code.c)
target_link_libraries(test_dangerous m)

add_executable(test_determinism_demo test_determinism.c)
target_link_libraries(test_determinism_demo m)

# Error impossibility verifier
add_executable(error_verifier 
    src/error_impossibility_verifier.c
    src/ast_analyzer.c
    src/determinism_analyzer.c
    src/constraint_verifier.c
)
target_link_libraries(error_verifier m)

# Dataflow prover
add_executable(dataflow_prover src/dataflow_prover.c)
target_link_libraries(dataflow_prover m)

# Library version for integration
add_library(ast_proof_lib STATIC
    src/ast_analyzer.c
    src/determinism_analyzer.c
    src/constraint_verifier.c
)
target_link_libraries(ast_proof_lib m)

# Custom targets for analysis
add_custom_target(analyze
    COMMAND $<TARGET_FILE:firewall-fly-over> -a ${CMAKE_CURRENT_SOURCE_DIR}/test_dangerous_code.c
    DEPENDS firewall-fly-over
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running AST safety analysis"
)

add_custom_target(prove
    COMMAND $<TARGET_FILE:firewall-fly-over> -f ${CMAKE_CURRENT_SOURCE_DIR}/test_determinism.c
    DEPENDS firewall-fly-over
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running full deterministic proof"
)

# Generate proof for any C file
function(add_proof_verification target source_file)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND $<TARGET_FILE:firewall-fly-over> -f ${source_file}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Verifying safety of ${source_file}"
    )
endfunction()

# Test the system
enable_testing()

add_test(NAME ast_analyzer_test
    COMMAND $<TARGET_FILE:firewall-fly-over> ${CMAKE_CURRENT_SOURCE_DIR}/test_dangerous_code.c
)

add_test(NAME determinism_test
    COMMAND $<TARGET_FILE:firewall-fly-over> -f ${CMAKE_CURRENT_SOURCE_DIR}/test_determinism.c
)

# Installation
install(TARGETS firewall-fly-over ast_proof_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/ast_proof_system
)

# Integration support
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ASTProofConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ASTProofConfig.cmake
    @ONLY
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ASTProofConfig.cmake
    DESTINATION lib/cmake/ASTProof
)

# Print configuration
message(STATUS "=== AST Proof System Configuration ===")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Clang: ${CLANG_EXECUTABLE}")
message(STATUS "GDB: ${GDB_EXECUTABLE}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Build with:")
message(STATUS "  make          - Build everything")
message(STATUS "  make analyze  - Run analysis demo")
message(STATUS "  make prove    - Run full proof demo")
message(STATUS "  make test     - Run tests")
message(STATUS "====================================")
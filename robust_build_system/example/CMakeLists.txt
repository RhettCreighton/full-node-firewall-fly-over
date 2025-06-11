# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)
project(SecureFirefoxApp VERSION 1.0.0 LANGUAGES C)

# Use our robust build system
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

# Include all modules
include(CompileTimeProof)
include(C99Compliance)  
include(BuildOptimization)
include(SafetyChecks)
include(SingleBinary)

# Configure build options
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")

# Enable all safety features
option(ENABLE_ALL_SAFETY "Enable all safety checks" ON)

# Source files
set(SOURCES
    src/main.c
    src/firefox_renderer.c
    src/safety_utils.c
)

# Create our single verified executable
add_verified_executable(SecureFirefoxApp
    SOURCES ${SOURCES}
    
    # Compile-time proof requirements
    PROOF_FUNCTION firefox_draw_pixel
    EXPECTED_OUTPUT "Drawing Firefox pixel"
    
    # Required invariants
    INVARIANTS
        firefox_draw_pixel      # Must call this function
        validate_pixel_bounds   # Must validate bounds
    
    # Safety requirements
    REQUIRED_SYMBOLS
        __stack_chk_fail       # Stack protection
        safe_malloc            # Memory safety
    
    FORBIDDEN_SYMBOLS
        strcpy                 # Unsafe functions
        gets
        sprintf
)

# Apply all safety and optimization features
apply_c99_compliance(SecureFirefoxApp)
apply_safety_checks(SecureFirefoxApp)
add_runtime_assertions(SecureFirefoxApp)
add_memory_safety_checks(SecureFirefoxApp)
add_integer_overflow_checks(SecureFirefoxApp)
configure_lto(SecureFirefoxApp)
optimize_binary_size(SecureFirefoxApp)

# Enable static analysis
add_static_analysis(SecureFirefoxApp)

# Configure optimizations
configure_optimization_flags()
configure_parallel_build()

# Installation
install(TARGETS SecureFirefoxApp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Print configuration summary
message(STATUS "")
message(STATUS "=== Secure Firefox App Configuration ===")
print_optimization_summary()
print_binary_summary()
message(STATUS "✓ Compile-time proofs enabled")
message(STATUS "✓ C99 compliance enforced")
message(STATUS "✓ Memory safety enabled")
message(STATUS "✓ Single binary constraint active")
message(STATUS "=========================================")
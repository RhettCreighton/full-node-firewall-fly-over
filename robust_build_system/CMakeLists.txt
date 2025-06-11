# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)
project(RobustBuildSystem VERSION 1.0.0 LANGUAGES C)

# Enforce C99 standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Include our modular build system
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Core modules
include(CompileTimeProof)
include(C99Compliance)
include(BuildOptimization)
include(SafetyChecks)
include(SingleBinary)

# Project configuration options
option(ENABLE_PROOF_SYSTEM "Enable compile-time proof verification" ON)
option(ENABLE_RUNTIME_CHECKS "Enable runtime safety checks" ON)
option(ENABLE_LTO "Enable Link Time Optimization" ON)
option(ENABLE_NATIVE_ARCH "Optimize for native architecture" OFF)
option(FORCE_SINGLE_BINARY "Ensure only one binary is produced" ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Global compiler flags for safety and optimization
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wold-style-definition
    -Wdeclaration-after-statement  # Enforce C99 style
    -Wvla  # Warn about VLAs
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
)

# Debug-specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g3 -O0 -fno-omit-frame-pointer)
    add_definitions(-DDEBUG_BUILD)
else()
    add_compile_options(-O3 -DNDEBUG)
endif()

# Architecture-specific optimizations
if(ENABLE_NATIVE_ARCH)
    add_compile_options(-march=native -mtune=native)
endif()

# Link-time optimization
if(ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# Export compilation database for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Function to add a verified executable
function(add_verified_executable TARGET)
    cmake_parse_arguments(AVE 
        ""  # Options
        "PROOF_FUNCTION;PROOF_OUTPUT"  # One-value args
        "SOURCES;LIBRARIES;INVARIANTS"  # Multi-value args
        ${ARGN}
    )
    
    # Create the executable
    add_executable(${TARGET} ${AVE_SOURCES})
    
    # Apply C99 compliance check
    apply_c99_compliance(${TARGET})
    
    # Apply safety checks
    apply_safety_checks(${TARGET})
    
    # Link libraries if provided
    if(AVE_LIBRARIES)
        target_link_libraries(${TARGET} PRIVATE ${AVE_LIBRARIES})
    endif()
    
    # Add compile-time proof if enabled
    if(ENABLE_PROOF_SYSTEM AND AVE_PROOF_FUNCTION)
        add_compile_time_proof(${TARGET}
            PROOF_FUNCTION ${AVE_PROOF_FUNCTION}
            EXPECTED_OUTPUT ${AVE_PROOF_OUTPUT}
            INVARIANTS ${AVE_INVARIANTS}
        )
    endif()
    
    # Ensure single binary constraint
    if(FORCE_SINGLE_BINARY)
        enforce_single_binary(${TARGET})
    endif()
endfunction()

# Installation rules
include(GNUInstallDirs)

# Package configuration
include(CMakePackageConfigHelpers)

# Create a template for new projects
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectTemplate.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ProjectTemplate.cmake"
    @ONLY
)

# Install the build system modules
install(
    DIRECTORY cmake/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/robust_build_system/cmake
    FILES_MATCHING PATTERN "*.cmake"
)

# Install the project template
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/ProjectTemplate.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/robust_build_system
)

# Create and install pkg-config file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/robust-build-system.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/robust-build-system.pc"
    @ONLY
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/robust-build-system.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
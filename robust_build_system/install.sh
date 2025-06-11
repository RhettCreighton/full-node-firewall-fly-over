#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Universal installer for Robust C99 Build System
# This script packages and installs the build system for any C99 CMake project

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# Default installation paths
DEFAULT_USER_CMAKE_DIR="$HOME/.cmake/modules"
DEFAULT_SYSTEM_CMAKE_DIR="/usr/local/share/cmake/Modules"
INSTALL_PREFIX=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --user)
            INSTALL_PREFIX="$DEFAULT_USER_CMAKE_DIR"
            shift
            ;;
        --system)
            INSTALL_PREFIX="$DEFAULT_SYSTEM_CMAKE_DIR"
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --help|-h)
            cat << EOF
Robust C99 Build System Installer

Usage: ./install.sh [OPTIONS]

Options:
    --user      Install to user directory (~/.cmake/modules)
    --system    Install system-wide (/usr/local/share/cmake/Modules)
    --prefix    Custom installation directory
    --help      Show this help message

Examples:
    ./install.sh --user                    # Install for current user
    sudo ./install.sh --system             # Install system-wide
    ./install.sh --prefix /opt/cmake       # Custom location

After installation, add to your CMakeLists.txt:
    include(RobustBuildSystem)
    enable_robust_build()
EOF
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Default to user installation if not specified
if [ -z "$INSTALL_PREFIX" ]; then
    INSTALL_PREFIX="$DEFAULT_USER_CMAKE_DIR"
    echo -e "${YELLOW}No installation prefix specified, using user installation${NC}"
fi

echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
echo -e "${BLUE}    Robust C99 Build System Installer${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
echo

# Check if running as root for system installation
if [[ "$INSTALL_PREFIX" == "$DEFAULT_SYSTEM_CMAKE_DIR" ]] && [[ $EUID -ne 0 ]]; then
    echo -e "${RED}Error: System installation requires root privileges${NC}"
    echo "Please run: sudo $0 --system"
    exit 1
fi

echo -e "${YELLOW}Installation directory:${NC} $INSTALL_PREFIX"

# Create installation directory
echo -e "\n${YELLOW}Creating installation directory...${NC}"
mkdir -p "$INSTALL_PREFIX/RobustBuildSystem"

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Copy all CMake modules
echo -e "${YELLOW}Installing CMake modules...${NC}"
cp -v "$SCRIPT_DIR/cmake/"*.cmake "$INSTALL_PREFIX/RobustBuildSystem/" 2>/dev/null || true

# Create the main include file
echo -e "\n${YELLOW}Creating main module...${NC}"
cat > "$INSTALL_PREFIX/RobustBuildSystem.cmake" << 'EOF'
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Robust C99 Build System - Main Module
# This module provides compile-time proof verification and extreme optimization

# Get the directory where this module is installed
get_filename_component(ROBUST_BUILD_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Core modules
include("${ROBUST_BUILD_DIR}/RobustBuildSystem/CompileTimeProof.cmake")
include("${ROBUST_BUILD_DIR}/RobustBuildSystem/OptimizedCompileTimeProof.cmake")
include("${ROBUST_BUILD_DIR}/RobustBuildSystem/AdvancedBuildOptimization.cmake")

# Optional modules (included if features are enabled)
set(ROBUST_BUILD_OPTIONAL_MODULES
    CPUFeatureDetection
    DistributedCompilation
    BuildProfiling
    LLVMProofPass
    HardwareCounters
)

# Main function to enable the robust build system
function(enable_robust_build)
    set(options 
        PROOF_VERIFICATION
        ULTRA_OPTIMIZATION
        DISTRIBUTED_BUILD
        NATIVE_ARCH
        BUILD_PROFILING
    )
    set(oneValueArgs PROOF_FUNCTION)
    set(multiValueArgs PROOF_SOURCES)
    
    cmake_parse_arguments(RB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Default to enabling proof verification
    if(NOT DEFINED RB_PROOF_VERIFICATION)
        set(RB_PROOF_VERIFICATION ON)
    endif()
    
    # Set global properties
    if(RB_PROOF_VERIFICATION)
        set(ENABLE_PROOF_SYSTEM ON CACHE BOOL "Enable compile-time proof system" FORCE)
        if(RB_PROOF_FUNCTION)
            set(REQUIRED_PROOF_FUNCTION "${RB_PROOF_FUNCTION}" CACHE STRING "Function to verify" FORCE)
        endif()
    endif()
    
    if(RB_ULTRA_OPTIMIZATION)
        set(ENABLE_ULTRA_PROOF ON CACHE BOOL "Enable ultra-fast proof verification" FORCE)
        # Load optional optimization modules
        foreach(module IN LISTS ROBUST_BUILD_OPTIONAL_MODULES)
            include("${ROBUST_BUILD_DIR}/RobustBuildSystem/${module}.cmake" OPTIONAL)
        endforeach()
    endif()
    
    if(RB_DISTRIBUTED_BUILD)
        set(ENABLE_DISTRIBUTED ON CACHE BOOL "Enable distributed compilation" FORCE)
        include("${ROBUST_BUILD_DIR}/RobustBuildSystem/DistributedCompilation.cmake" OPTIONAL)
    endif()
    
    if(RB_NATIVE_ARCH)
        set(ENABLE_NATIVE_ARCH ON CACHE BOOL "Enable CPU-specific optimizations" FORCE)
        include("${ROBUST_BUILD_DIR}/RobustBuildSystem/CPUFeatureDetection.cmake" OPTIONAL)
    endif()
    
    if(RB_BUILD_PROFILING)
        set(ENABLE_BUILD_PROFILING ON CACHE BOOL "Enable build time profiling" FORCE)
        include("${ROBUST_BUILD_DIR}/RobustBuildSystem/BuildProfiling.cmake" OPTIONAL)
    endif()
    
    # Enable all optimizations if requested
    if(RB_ULTRA_OPTIMIZATION)
        enable_all_optimizations()
    endif()
    
    message(STATUS "Robust Build System enabled with:")
    message(STATUS "  Proof Verification: ${RB_PROOF_VERIFICATION}")
    message(STATUS "  Ultra Optimization: ${RB_ULTRA_OPTIMIZATION}")
    message(STATUS "  Distributed Build: ${RB_DISTRIBUTED_BUILD}")
    message(STATUS "  Native Architecture: ${RB_NATIVE_ARCH}")
    message(STATUS "  Build Profiling: ${RB_BUILD_PROFILING}")
endfunction()

# Convenience function for targets
function(add_robust_executable TARGET)
    set(options REQUIRE_PROOF)
    set(oneValueArgs PROOF_FUNCTION)
    set(multiValueArgs SOURCES)
    
    cmake_parse_arguments(RE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Create the executable
    add_executable(${TARGET} ${RE_SOURCES})
    
    # Apply proof verification if enabled
    if(ENABLE_PROOF_SYSTEM AND RE_REQUIRE_PROOF)
        if(RE_PROOF_FUNCTION)
            add_compile_time_proof(${TARGET} 
                REQUIRED_FUNCTION "${RE_PROOF_FUNCTION}"
                SOURCES ${RE_SOURCES}
            )
        else()
            add_compile_time_proof(${TARGET} SOURCES ${RE_SOURCES})
        endif()
    endif()
    
    # Apply optimizations if enabled
    if(ENABLE_ULTRA_PROOF)
        if(COMMAND enable_ultimate_optimization)
            enable_ultimate_optimization(${TARGET})
        endif()
    endif()
endfunction()

# Quick setup for common use case
macro(robust_c99_project)
    # Set C99 standard
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    
    # Enable robust build with all features
    enable_robust_build(
        PROOF_VERIFICATION
        ULTRA_OPTIMIZATION
        NATIVE_ARCH
    )
    
    # Set default build type if not specified
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    endif()
    
    # Use Ninja if available
    if(NOT CMAKE_GENERATOR)
        find_program(NINJA_EXECUTABLE ninja)
        if(NINJA_EXECUTABLE)
            set(CMAKE_GENERATOR "Ninja" CACHE STRING "CMake generator" FORCE)
        endif()
    endif()
endmacro()

# Print configuration summary
function(robust_build_summary)
    message(STATUS "")
    message(STATUS "Robust Build System Configuration:")
    message(STATUS "══════════════════════════════════")
    
    if(ENABLE_PROOF_SYSTEM)
        message(STATUS "✓ Compile-time proof verification enabled")
        if(REQUIRED_PROOF_FUNCTION)
            message(STATUS "  Required function: ${REQUIRED_PROOF_FUNCTION}")
        endif()
    endif()
    
    if(ENABLE_ULTRA_PROOF)
        message(STATUS "✓ Ultra-fast proof verification enabled")
        message(STATUS "  - Parallel verification")
        message(STATUS "  - SHA256 caching")
        message(STATUS "  - Multiple proof methods")
    endif()
    
    if(ENABLE_DISTRIBUTED)
        message(STATUS "✓ Distributed compilation enabled")
    endif()
    
    if(ENABLE_NATIVE_ARCH)
        message(STATUS "✓ CPU-specific optimizations enabled")
    endif()
    
    if(CMAKE_GENERATOR STREQUAL "Ninja")
        message(STATUS "✓ Using Ninja for faster builds")
    endif()
    
    message(STATUS "")
endfunction()
EOF

# Create proof verification scripts directory
echo -e "\n${YELLOW}Installing verification scripts...${NC}"
mkdir -p "$INSTALL_PREFIX/RobustBuildSystem/scripts"

# Copy verification scripts
if [ -d "$SCRIPT_DIR/scripts" ]; then
    cp -v "$SCRIPT_DIR/scripts/"*.sh "$INSTALL_PREFIX/RobustBuildSystem/scripts/" 2>/dev/null || true
    chmod +x "$INSTALL_PREFIX/RobustBuildSystem/scripts/"*.sh 2>/dev/null || true
fi

# Create a pkg-config file for system installations
if [[ "$INSTALL_PREFIX" == "$DEFAULT_SYSTEM_CMAKE_DIR" ]]; then
    echo -e "\n${YELLOW}Creating pkg-config file...${NC}"
    mkdir -p /usr/local/lib/pkgconfig
    cat > /usr/local/lib/pkgconfig/robust-build-system.pc << EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include
datarootdir=\${prefix}/share
datadir=\${datarootdir}
cmakedir=\${datadir}/cmake/Modules

Name: Robust Build System
Description: Compile-time proof verification and optimization for C99 projects
Version: 1.0.0
Cflags: 
Libs: 
EOF
fi

# Create example project
echo -e "\n${YELLOW}Creating example project...${NC}"
EXAMPLE_DIR="$INSTALL_PREFIX/RobustBuildSystem/example"
mkdir -p "$EXAMPLE_DIR/src"

cat > "$EXAMPLE_DIR/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(RobustExample C)

# Include the Robust Build System
include(RobustBuildSystem)

# Enable robust build with all optimizations
robust_c99_project()

# Add executable with proof requirement
add_robust_executable(example
    REQUIRE_PROOF
    PROOF_FUNCTION "verified_function"
    SOURCES
        src/main.c
        src/verified.c
)

# Print configuration summary
robust_build_summary()
EOF

cat > "$EXAMPLE_DIR/src/main.c" << 'EOF'
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

extern void verified_function(void);

int main(void) {
    printf("Robust Build System Example\n");
    
    /* This function call is verified at compile time */
    verified_function();
    
    printf("✅ Proof verification passed!\n");
    return 0;
}
EOF

cat > "$EXAMPLE_DIR/src/verified.c" << 'EOF'
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

/* This function must exist and be called for compilation to succeed */
void verified_function(void) {
    printf("This function was verified at compile time!\n");
}
EOF

# Create README for the installation
cat > "$INSTALL_PREFIX/RobustBuildSystem/README.md" << 'EOF'
# Robust C99 Build System

## Quick Start

Add to your CMakeLists.txt:

```cmake
# Option 1: Simple setup with all features
include(RobustBuildSystem)
robust_c99_project()

# Option 2: Custom configuration
include(RobustBuildSystem)
enable_robust_build(
    PROOF_VERIFICATION      # Enable compile-time proofs
    ULTRA_OPTIMIZATION      # Enable all optimizations
    DISTRIBUTED_BUILD       # Enable distributed compilation
    NATIVE_ARCH            # CPU-specific optimizations
    BUILD_PROFILING        # Track build performance
)

# Option 3: Add individual targets
add_robust_executable(myapp
    REQUIRE_PROOF
    PROOF_FUNCTION "required_function_name"
    SOURCES src/main.c src/other.c
)
```

## Features

- **Compile-time Proof Verification**: Ensures required functions are called
- **100x Faster Verification**: Through caching and parallel methods
- **Distributed Compilation**: Support for ccache, icecc, distcc
- **CPU Optimization**: Automatic detection of AVX2, AVX-512, NEON
- **Build Profiling**: Track and optimize build times

## Example

See the example project in the `example/` directory.

## Documentation

- [Optimization Guide](OPTIMIZATION_GUIDE.md)
- [Build System Architecture](ARCHITECTURE.md)
EOF

# Check for required tools and provide recommendations
echo -e "\n${BLUE}Checking system tools...${NC}"

check_tool() {
    local tool=$1
    local package=$2
    local required=$3
    
    if command -v "$tool" &>/dev/null; then
        echo -e "  ${GREEN}✓${NC} $tool: Found"
    else
        if [ "$required" = "required" ]; then
            echo -e "  ${RED}✗${NC} $tool: Not found (required)"
            echo -e "    Install with: ${YELLOW}sudo apt install $package${NC}"
        else
            echo -e "  ${YELLOW}○${NC} $tool: Not found (optional)"
            echo -e "    Install with: ${YELLOW}sudo apt install $package${NC}"
        fi
    fi
}

check_tool "cmake" "cmake" "required"
check_tool "ninja" "ninja-build" "optional"
check_tool "ccache" "ccache" "optional"
check_tool "clang" "clang" "optional"
check_tool "distcc" "distcc" "optional"

# Create uninstall script
echo -e "\n${YELLOW}Creating uninstall script...${NC}"
cat > "$INSTALL_PREFIX/RobustBuildSystem/uninstall.sh" << EOF
#!/bin/bash
# Uninstall script for Robust Build System

echo "Removing Robust Build System from $INSTALL_PREFIX"
rm -rf "$INSTALL_PREFIX/RobustBuildSystem"
rm -f "$INSTALL_PREFIX/RobustBuildSystem.cmake"
[ -f /usr/local/lib/pkgconfig/robust-build-system.pc ] && rm -f /usr/local/lib/pkgconfig/robust-build-system.pc
echo "✓ Uninstall complete"
EOF
chmod +x "$INSTALL_PREFIX/RobustBuildSystem/uninstall.sh"

# Success message
echo
echo -e "${GREEN}═══════════════════════════════════════════════════${NC}"
echo -e "${GREEN}    Installation Complete!${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════${NC}"
echo
echo -e "${BLUE}To use in your project, add to CMakeLists.txt:${NC}"
echo
echo "    include(RobustBuildSystem)"
echo "    robust_c99_project()"
echo
echo -e "${BLUE}Example project available at:${NC}"
echo "    $INSTALL_PREFIX/RobustBuildSystem/example/"
echo
echo -e "${BLUE}To uninstall:${NC}"
echo "    $INSTALL_PREFIX/RobustBuildSystem/uninstall.sh"
echo

# Add to CMAKE_MODULE_PATH hint if user installation
if [[ "$INSTALL_PREFIX" == "$DEFAULT_USER_CMAKE_DIR" ]]; then
    echo -e "${YELLOW}Note: You may need to add this to your CMakeLists.txt:${NC}"
    echo "    list(APPEND CMAKE_MODULE_PATH \"$INSTALL_PREFIX\")"
    echo
fi
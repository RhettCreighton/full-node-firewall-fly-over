#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Initialize a new project with the robust build system

set -e

PROJECT_NAME="$1"
PROJECT_DIR="$2"

if [ -z "$PROJECT_NAME" ] || [ -z "$PROJECT_DIR" ]; then
    echo "Usage: $0 <project_name> <project_directory>"
    echo "Example: $0 MySecureApp ~/projects/my_secure_app"
    exit 1
fi

echo "=== Robust Build System Project Initializer ==="
echo "Creating project: $PROJECT_NAME"
echo "Location: $PROJECT_DIR"
echo ""

# Create project structure
mkdir -p "$PROJECT_DIR"/{src,include,tests,docs,build}

# Copy cmake modules
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cp -r "$SCRIPT_DIR/cmake" "$PROJECT_DIR/"

# Create main CMakeLists.txt
cat > "$PROJECT_DIR/CMakeLists.txt" << EOF
# SPDX-FileCopyrightText: $(date +%Y) Your Name
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)
project($PROJECT_NAME VERSION 1.0.0 LANGUAGES C)

# Use the robust build system
list(APPEND CMAKE_MODULE_PATH "\${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include all modules
include(CompileTimeProof)
include(C99Compliance)
include(BuildOptimization)
include(SafetyChecks)
include(SingleBinary)

# Configure C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build options
option(ENABLE_PROOF_SYSTEM "Enable compile-time proof verification" ON)
option(ENABLE_ALL_SAFETY "Enable all safety checks" ON)
option(ENABLE_STATIC_ANALYSIS "Enable static analysis tools" ON)

# Source files
set(SOURCES
    src/main.c
    # Add your source files here
)

# Create the single verified executable
add_verified_executable($PROJECT_NAME
    SOURCES \${SOURCES}
    
    # Define your proof requirements here
    # PROOF_FUNCTION your_required_function
    # EXPECTED_OUTPUT "Expected output"
    
    # INVARIANTS
    #     function_that_must_be_called
    
    # REQUIRED_SYMBOLS
    #     __stack_chk_fail
    
    # FORBIDDEN_SYMBOLS
    #     strcpy
    #     gets
)

# Apply safety and optimization
apply_c99_compliance($PROJECT_NAME)
apply_safety_checks($PROJECT_NAME)
add_runtime_assertions($PROJECT_NAME)
add_memory_safety_checks($PROJECT_NAME)
configure_lto($PROJECT_NAME)

# Enable static analysis
if(ENABLE_STATIC_ANALYSIS)
    add_static_analysis($PROJECT_NAME)
endif()

# Configure build optimizations
configure_optimization_flags()
configure_parallel_build()

# Installation
install(TARGETS $PROJECT_NAME
    RUNTIME DESTINATION \${CMAKE_INSTALL_BINDIR}
)

# Print configuration
message(STATUS "")
message(STATUS "=== $PROJECT_NAME Configuration ===")
print_optimization_summary()
print_binary_summary()
EOF

# Create a simple main.c
cat > "$PROJECT_DIR/src/main.c" << 'EOF'
/* SPDX-FileCopyrightText: 2025 Your Name
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

int main(void) {
    printf("Hello from %s!\n", PROJECT_NAME);
    printf("Built with the Robust Build System\n");
    
    /* Your secure C99 code here */
    
    return 0;
}
EOF

# Create build script
cat > "$PROJECT_DIR/build.sh" << 'EOF'
#!/bin/bash
# Quick build script

set -e

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build with parallel jobs
cmake --build . -j$(nproc)

echo ""
echo "Build complete! Binary: build/$PROJECT_NAME"
EOF

chmod +x "$PROJECT_DIR/build.sh"

# Create README
cat > "$PROJECT_DIR/README.md" << EOF
# $PROJECT_NAME

Built with the Robust C99 CMake Build System.

## Features

- ✓ Compile-time proof verification
- ✓ C99 standard compliance
- ✓ Memory safety checks
- ✓ Integer overflow protection
- ✓ Static analysis integration
- ✓ Single binary output
- ✓ Build optimization

## Building

\`\`\`bash
./build.sh
\`\`\`

Or manually:

\`\`\`bash
mkdir build
cd build
cmake ..
make -j\$(nproc)
\`\`\`

## Configuration Options

- \`ENABLE_PROOF_SYSTEM\`: Enable compile-time proofs (default: ON)
- \`ENABLE_ALL_SAFETY\`: Enable all safety checks (default: ON)
- \`CMAKE_BUILD_TYPE\`: Debug, Release, RelWithDebInfo, MinSizeRel

## Adding Compile-Time Proofs

Edit CMakeLists.txt and add your requirements:

\`\`\`cmake
add_verified_executable($PROJECT_NAME
    SOURCES \${SOURCES}
    PROOF_FUNCTION my_critical_function
    EXPECTED_OUTPUT "Function called"
    INVARIANTS my_critical_function
)
\`\`\`
EOF

# Create .gitignore
cat > "$PROJECT_DIR/.gitignore" << 'EOF'
# Build directory
build/
*.build/

# Generated files
*.o
*.a
*.so
*.exe

# CMake files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
compile_commands.json

# Editor files
.vscode/
.idea/
*.swp
*~
EOF

echo ""
echo "✓ Project initialized successfully!"
echo ""
echo "Next steps:"
echo "  cd $PROJECT_DIR"
echo "  ./build.sh"
echo ""
echo "To add compile-time proofs, edit CMakeLists.txt"
echo "and define your PROOF_FUNCTION and INVARIANTS."
echo ""
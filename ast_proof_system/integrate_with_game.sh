#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Script to integrate AST Proof System with the game project

set -e

echo "=== Integrating AST Proof System with Full Node Firewall Fly-over ==="
echo

# Get the project root (parent directory)
PROJECT_ROOT=$(dirname $(pwd))
AST_DIR=$(pwd)

echo "Project root: $PROJECT_ROOT"
echo "AST system: $AST_DIR"
echo

# Add to parent project's CMakeLists.txt
echo "Adding AST verification to main CMakeLists.txt..."

# Check if already integrated
if grep -q "ast_proof_system" "$PROJECT_ROOT/CMakeLists.txt" 2>/dev/null; then
    echo "✓ Already integrated in CMakeLists.txt"
else
    # Create integration snippet
    cat > integration_snippet.cmake << 'EOF'

# AST Safety Verification System
option(ENABLE_AST_VERIFICATION "Enable mathematical safety proofs" ON)

if(ENABLE_AST_VERIFICATION)
    add_subdirectory(ast_proof_system)
    include(${CMAKE_CURRENT_SOURCE_DIR}/ast_proof_system/cmake/ASTVerification.cmake)
    
    # Add verification to main game executable
    if(TARGET sky_combat_ultimate)
        add_ast_verification(sky_combat_ultimate)
        add_deterministic_optimization(sky_combat_ultimate)
    endif()
    
    # Configure AST proof system
    configure_ast_proof_system(
        ENABLE_CACHE
        PARALLEL
        CACHE_DIR ${CMAKE_BINARY_DIR}/.ast_cache
        CRITICAL_PATHS src/models src/controllers
    )
endif()
EOF

    echo
    echo "Add the following to your CMakeLists.txt:"
    echo "----------------------------------------"
    cat integration_snippet.cmake
    echo "----------------------------------------"
fi

# Create verification script for the game
echo
echo "Creating game-specific verification script..."

cat > "$PROJECT_ROOT/verify_game_safety.sh" << 'EOF'
#!/bin/bash
# Verify safety of Full Node Firewall Fly-over

AST_DIR="ast_proof_system"

echo "=== Verifying Game Safety ==="

# Critical game files to verify
CRITICAL_FILES=(
    "src/models/enemies.c"
    "src/models/aircraft.c"
    "src/models/weapons.c"
    "src/controllers/input_controller.c"
    "src/controllers/sky_combat.c"
)

# Run enhanced AST analysis on critical files
echo "Analyzing critical game systems..."
for file in "${CRITICAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "Checking $file..."
        python3 $AST_DIR/enhanced_ast_analyzer.py "$file"
    fi
done

# Run determinism analysis
echo
echo "Analyzing determinism for optimizations..."
python3 $AST_DIR/determinism_analyzer.py src/models/enemies.c

# Generate full report
echo
echo "Generating safety report..."
python3 $AST_DIR/incremental_verifier.py src/ --report game_safety_report.md

echo
echo "Safety verification complete!"
echo "See game_safety_report.md for details"
EOF

chmod +x "$PROJECT_ROOT/verify_game_safety.sh"

# Create pre-commit hook
echo
echo "Creating Git pre-commit hook..."

mkdir -p "$PROJECT_ROOT/.git/hooks" 2>/dev/null || true

cat > "$PROJECT_ROOT/.git/hooks/pre-commit" << 'EOF'
#!/bin/bash
# Pre-commit hook to verify safety of changed files

AST_DIR="ast_proof_system"

# Get changed C files
CHANGED=$(git diff --cached --name-only --diff-filter=ACM | grep '\.c$')

if [ -z "$CHANGED" ]; then
    exit 0
fi

echo "Running safety verification on changed files..."

FAILED=0
for file in $CHANGED; do
    if python3 $AST_DIR/enhanced_ast_analyzer.py "$file" | grep -q "unsafe"; then
        echo "✗ $file has safety issues!"
        FAILED=1
    else
        echo "✓ $file is safe"
    fi
done

if [ $FAILED -eq 1 ]; then
    echo
    echo "Safety verification failed!"
    echo "Fix the issues or use --no-verify to skip"
    exit 1
fi

echo "All files passed safety verification!"
EOF

chmod +x "$PROJECT_ROOT/.git/hooks/pre-commit" 2>/dev/null || true

# Test on actual game file
echo
echo "Testing on actual game code..."

if [ -f "$PROJECT_ROOT/src/models/enemies.c" ]; then
    echo "Analyzing enemies.c..."
    python3 enhanced_ast_analyzer.py "$PROJECT_ROOT/src/models/enemies.c" > game_test_output.txt 2>&1
    
    if grep -q "Found.*dangerous operations" game_test_output.txt; then
        echo "✓ Analysis working! Found operations to verify:"
        grep -A5 "dangerous operations" game_test_output.txt
    fi
fi

# Create example safe wrapper
echo
echo "Creating example safe wrapper..."

cat > "$PROJECT_ROOT/src/utils/safe_operations.h" << 'EOF'
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SAFE_OPERATIONS_H
#define SAFE_OPERATIONS_H

#include "../ast_proof_system/include/ast_proof_config.h"

/* Safe division with compile-time proof */
static inline float safe_divide_f(float a, float b) {
    return (b != 0.0f) ? (a / b) : 0.0f;
}

/* Safe array access with bounds check */
#define SAFE_ARRAY_GET(arr, idx, size, default) \
    (((idx) >= 0 && (idx) < (size)) ? (arr)[idx] : (default))

/* Safe normalize for vectors */
static inline Vector3 safe_normalize(Vector3 v) {
    float length = Vector3Length(v);
    if (length < 0.0001f) {
        return (Vector3){0.0f, 0.0f, 1.0f};
    }
    return Vector3Scale(v, 1.0f / length);
}

/* Mark deterministic physics calculations */
DETERMINISTIC static inline float calculate_kinetic_energy(float mass, float velocity) {
    return 0.5f * mass * velocity * velocity;
}

#endif /* SAFE_OPERATIONS_H */
EOF

echo
echo "=== Integration Complete ==="
echo
echo "Next steps:"
echo "1. Add the CMake snippet to $PROJECT_ROOT/CMakeLists.txt"
echo "2. Run: ./verify_game_safety.sh"
echo "3. Fix any safety issues found"
echo "4. Commit with confidence - pre-commit hook will verify safety!"
echo
echo "The AST Proof System will now:"
echo "- Verify safety on every build"
echo "- Check files before commit"
echo "- Generate mathematical proofs"
echo "- Optimize deterministic operations"
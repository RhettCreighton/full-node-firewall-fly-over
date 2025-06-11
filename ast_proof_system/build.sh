#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Simple build script for AST Proof System

set -e

echo "=== Building AST GDB Deterministic Proving System ==="
echo

# Check dependencies
echo "Checking dependencies..."

check_command() {
    if command -v "$1" > /dev/null 2>&1; then
        echo "✓ $1 found"
        return 0
    else
        echo "✗ $1 not found"
        return 1
    fi
}

MISSING_DEPS=0

check_command python3 || MISSING_DEPS=1
check_command clang || MISSING_DEPS=1
check_command gdb || MISSING_DEPS=1
check_command cmake || MISSING_DEPS=1

if [ $MISSING_DEPS -eq 1 ]; then
    echo
    echo "Missing dependencies detected!"
    echo "Install on Ubuntu/Debian:"
    echo "  sudo apt-get install python3 clang gdb cmake"
    echo
    echo "Install on macOS:"
    echo "  brew install python3 llvm gdb cmake"
    exit 1
fi

# Check Python dependencies
echo
echo "Checking Python dependencies..."
if python3 -c "import z3" 2>/dev/null; then
    echo "✓ z3-solver installed"
else
    echo "⚠ z3-solver not installed (optional but recommended)"
    echo "  Install with: pip install z3-solver"
fi

# Create build directory
echo
echo "Creating build directory..."
mkdir -p build
cd build

# Run CMake
echo
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo
echo "Building..."
make -j$(nproc 2>/dev/null || echo 4)

# Run tests
echo
echo "Running validation tests..."
if make validate; then
    echo "✓ All tests passed!"
else
    echo "⚠ Some tests failed (this may be expected for test files)"
fi

echo
echo "=== Build Complete ==="
echo
echo "Binaries are in: build/bin/"
echo "To test the system:"
echo "  cd build"
echo "  make prove"
echo
echo "To integrate with your project:"
echo "  See INTEGRATION_GUIDE.md"
echo
echo "Quick test:"
echo "  ./bin/enhanced_ast_analyzer.py ../test_dangerous_code.c"
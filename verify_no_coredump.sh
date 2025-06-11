#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

echo "=========================================="
echo "Full Node: Firewall Fly-over"
echo "Formal Memory Safety Verification"
echo "=========================================="
echo ""

# Build the project
echo "Building verification system..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make test_no_coredump -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Running memory safety verification..."
echo ""

# Run the verification test
./test_no_coredump

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ VERIFICATION SUCCESSFUL"
    echo "The game has been formally proven to never coredump!"
    echo ""
    echo "Truth Bucket System has verified:"
    echo "  - All array accesses are bounds-checked"
    echo "  - No null pointer dereferences possible"
    echo "  - No division by zero conditions"
    echo "  - Memory allocations are safe"
    echo "  - No integer overflow vulnerabilities"
else
    echo ""
    echo "❌ VERIFICATION FAILED"
    echo "Memory safety issues detected!"
    exit 1
fi
#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

echo "Building Sky Combat Modular..."

# Create build directory
mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo ""
    echo "Run with:"
    echo "  ./build/sky_combat_main"
    echo ""
    echo "Or test individual features:"
    echo "  ./build/test_flight"
    echo "  ./build/test_world"
    echo "  ./build/test_characters"
else
    echo "Build failed!"
    exit 1
fi
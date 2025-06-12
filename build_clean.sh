#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

echo "=== BUILDING CLEAN SKY COMBAT PROJECT ==="
echo

# Clean previous build
rm -rf build
mkdir -p build
cd build

# Configure
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo
echo "Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo
    echo "Build successful!"
    echo
    echo "Run with:"
    echo "  ./build/sky_combat"
    echo
    ls -la sky_combat
else
    echo "Build failed!"
    exit 1
fi
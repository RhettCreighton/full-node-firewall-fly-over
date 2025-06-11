#!/bin/bash
# Simple build script for Sky Combat game

set -e

echo "Building Sky Combat game..."

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make -j$(nproc)

echo "Build complete! Run with: ./run_ultimate.sh"
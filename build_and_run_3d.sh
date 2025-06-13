#!/bin/bash
# Build and run the Sky Combat 3D game using CMake

echo "Building Sky Combat 3D..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake (disable philosophical proofs for quick build)
cmake .. -DREQUIRE_PHILOSOPHICAL_PROOFS=OFF

# Build the 3D game executable
make sky_combat_3d_simple

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo ""
    echo "==================================="
    echo "  Sky Combat 3D - Cyberpunk City"
    echo "==================================="
    echo ""
    echo "Controls:"
    echo "  WASD - Move camera/aircraft"
    echo "  Q/E - Decrease/Increase speed"
    echo "  Space/Ctrl - Up/Down altitude"
    echo "  Mouse - Look around"
    echo "  ESC - Exit"
    echo ""
    echo "Starting game..."
    echo ""
    
    # Run the game
    ./sky_combat_3d_simple
else
    echo "Build failed! Please check for errors above."
fi
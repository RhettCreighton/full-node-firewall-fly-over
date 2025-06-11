#!/bin/bash
# Build with all safety checks enabled

set -e

echo "Building with safety sanitizers..."

mkdir -p build_safe
cd build_safe

# Configure with sanitizers
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined,float-divide-by-zero -fno-omit-frame-pointer -g"

# Build
make -j$(nproc)

echo "Safe build complete! Run with:"
echo "cd build_safe && ./sky_combat_multiplayer_incremental"
echo ""
echo "This build will detect:"
echo "- Buffer overflows"
echo "- Use after free"
echo "- Division by zero"
echo "- Integer overflow"
echo "- Null pointer dereference"
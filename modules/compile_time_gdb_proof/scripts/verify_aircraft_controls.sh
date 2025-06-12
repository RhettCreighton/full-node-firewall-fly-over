#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"

echo "=== Verifying Aircraft Controls Work ==="
echo "Building test binary..."

cd "$PROJECT_ROOT"

# Build with debug symbols
gcc -g -O0 -o test_aircraft_controls \
    src/sky_combat_aircraft_world.c \
    -lraylib -lm -lpthread \
    -I include \
    -I specifications \
    -DDEBUG

echo "Running GDB verification..."

# Run GDB with our verification script
gdb -batch -x "$SCRIPT_DIR/verify_aircraft_controls.gdb" test_aircraft_controls

if [ $? -eq 0 ]; then
    echo "✓ Aircraft controls verified to work!"
else
    echo "✗ Aircraft control verification failed!"
    exit 1
fi

# Cleanup
rm -f test_aircraft_controls

echo "Verification complete."
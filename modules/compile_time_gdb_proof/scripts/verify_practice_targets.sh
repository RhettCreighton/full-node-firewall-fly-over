#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Practice Targets Verification Script
# Verifies that:
# 1. Practice targets spawn in predictable patterns
# 2. Explosions are constrained to 1.5x enemy radius
# 3. Explosions use high contrast colors

BINARY="$1"
SCRIPT_DIR="$(dirname "$0")"

if [ ! -f "$BINARY" ]; then
    echo "❌ Error: Binary not found: $BINARY"
    exit 1
fi

echo "=== GDB Practice Targets Verification ==="
echo "Ensuring practice targets and controlled explosions..."
echo ""

# Run GDB with the verification script without opening window
GDB_VERIFICATION_MODE=1 timeout 10s gdb -batch -x "${SCRIPT_DIR}/verify_practice_targets.gdb" "$BINARY" 2>&1 | \
    grep -v "^Reading symbols" | \
    grep -v "^Downloading" | \
    grep -v "^This GDB" | \
    grep -v "^Debuginfod" | \
    grep -v "^To make this" | \
    grep -v "Thread debugging" | \
    grep -v "Using host" | \
    grep -v "^Function(s)" | \
    grep -v "^\[Thread" | \
    grep -v "^\[New Thread" | \
    grep -v "^INFO:" | \
    grep -v "^WARNING:"

# Check exit status
STATUS=${PIPESTATUS[0]}

if [ $STATUS -eq 0 ]; then
    echo ""
    echo "✅ Practice targets verified!"
    echo "🎯 Predictable patterns for practice"
    echo "💥 Controlled explosions (1.5x radius)"
    echo "🌈 High contrast explosion colors"
    exit 0
else
    echo ""
    echo "❌ BUILD BLOCKED: Practice targets not properly implemented!"
    echo "Requirements:"
    echo "1. enemies_spawn_practice_targets() must exist"
    echo "2. Explosions must be limited to 1.5x enemy radius"
    echo "3. Explosions must use high contrast colors"
    echo ""
    echo "To fix:"
    echo "1. Implement practice target patterns"
    echo "2. Control explosion radius"
    echo "3. Use high contrast colors per enemy type"
    exit 1
fi
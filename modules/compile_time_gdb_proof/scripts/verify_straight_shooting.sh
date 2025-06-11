#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Straight Shooting Verification Script
# Ensures that bullets shoot straight forward with no aim offset

BINARY="$1"
SCRIPT_DIR="$(dirname "$0")"

if [ ! -f "$BINARY" ]; then
    echo "❌ Error: Binary not found: $BINARY"
    exit 1
fi

echo "=== GDB Straight Shooting Verification ==="
echo "Ensuring bullets shoot straight forward..."
echo ""

# Run GDB with the verification script without opening window
GDB_VERIFICATION_MODE=1 timeout 10s gdb -batch -x "${SCRIPT_DIR}/verify_straight_shooting.gdb" "$BINARY" 2>&1 | \
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
    echo "✅ Straight shooting verified!"
    echo "➡️  Bullets fire straight forward"
    echo "🚫 No aim offset applied"
    exit 0
else
    echo ""
    echo "❌ BUILD BLOCKED: Straight shooting not verified!"
    echo "Bullets MUST shoot straight forward."
    echo ""
    echo "To fix:"
    echo "1. Remove any aim offset calculations"
    echo "2. Use aircraft forward vector only"
    echo "3. Call VERIFY_STRAIGHT_SHOOTING() in main()"
    exit 1
fi
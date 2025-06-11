#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Right Stick Unused Verification Script
# Ensures that right joystick has NO function in the game

BINARY="$1"
SCRIPT_DIR="$(dirname "$0")"

if [ ! -f "$BINARY" ]; then
    echo "❌ Error: Binary not found: $BINARY"
    exit 1
fi

echo "=== GDB Right Stick Unused Verification ==="
echo "Ensuring right stick has no function..."
echo ""

# Run GDB with the verification script without opening window
GDB_VERIFICATION_MODE=1 timeout 10s gdb -batch -x "${SCRIPT_DIR}/verify_right_stick_unused.gdb" "$BINARY" 2>&1 | \
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
    echo "✅ Right stick unused verified!"
    echo "🚫 Right stick has NO function"
    echo "📴 Input is completely ignored"
    exit 0
else
    echo ""
    echo "❌ BUILD BLOCKED: Right stick unused not verified!"
    echo "Right stick MUST have no function."
    echo ""
    echo "To fix:"
    echo "1. Ensure right stick input is not used"
    echo "2. Keep camera and aim code commented out"
    echo "3. Call VERIFY_RIGHT_STICK_UNUSED() in main()"
    exit 1
fi
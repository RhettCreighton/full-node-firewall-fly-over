#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Right Stick Aim Verification Script
# Ensures that right joystick controls gun aiming, NOT camera

BINARY="$1"
SCRIPT_DIR="$(dirname "$0")"

if [ ! -f "$BINARY" ]; then
    echo "❌ Error: Binary not found: $BINARY"
    exit 1
fi

echo "=== GDB Right Stick Aim Verification ==="
echo "Ensuring right stick controls gun aiming, not camera..."
echo ""

# Run GDB with the verification script without opening window
GDB_VERIFICATION_MODE=1 timeout 10s gdb -batch -x "${SCRIPT_DIR}/verify_right_stick_aim.gdb" "$BINARY" 2>&1 | \
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
    echo "✅ Right stick aim control verified!"
    echo "🎯 Right joystick fine-tunes gun spray"
    echo "🚫 Camera control FORBIDDEN on right stick"
    exit 0
else
    echo ""
    echo "❌ BUILD BLOCKED: Right stick aim not verified!"
    echo "Right stick MUST control gun aiming, NOT camera."
    echo ""
    echo "To fix:"
    echo "1. Remove camera control from right stick (axes 2 & 3)"
    echo "2. Implement gun aim offset using right stick"
    echo "3. Call VERIFY_RIGHT_STICK_AIM() in main()"
    exit 1
fi
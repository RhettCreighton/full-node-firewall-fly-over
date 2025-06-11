#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Gun Tuning Verification Script
# Ensures that BOTH conditions are true:
# 1. Right stick fine-tunes gun parameters
# 2. Bullets still shoot straight forward

BINARY="$1"
SCRIPT_DIR="$(dirname "$0")"

if [ ! -f "$BINARY" ]; then
    echo "❌ Error: Binary not found: $BINARY"
    exit 1
fi

echo "=== GDB Gun Tuning Verification ==="
echo "Ensuring right stick fine-tunes guns AND bullets shoot straight..."
echo ""

# Run GDB with the verification script without opening window
GDB_VERIFICATION_MODE=1 timeout 10s gdb -batch -x "${SCRIPT_DIR}/verify_gun_tuning.gdb" "$BINARY" 2>&1 | \
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
    echo "✅ Gun tuning system verified!"
    echo "🎮 Right stick fine-tunes gun parameters"
    echo "➡️  Bullets still shoot straight forward"
    echo "✨ BOTH conditions are TRUE"
    exit 0
else
    echo ""
    echo "❌ BUILD BLOCKED: Gun tuning not properly implemented!"
    echo "Requirements:"
    echo "1. Right stick MUST fine-tune gun parameters"
    echo "2. Bullets MUST still shoot straight forward"
    echo ""
    echo "To fix:"
    echo "1. Call weapons_set_fine_tuning() with right stick input"
    echo "2. Keep using forward vector for bullet direction"
    echo "3. Call VERIFY_GUN_TUNING() in main()"
    exit 1
fi
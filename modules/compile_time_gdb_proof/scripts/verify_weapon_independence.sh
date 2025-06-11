#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Weapon Independence Verification Script
# Ensures that weapon firing NEVER affects aircraft movement

BINARY="$1"
SCRIPT_DIR="$(dirname "$0")"

if [ ! -f "$BINARY" ]; then
    echo "❌ Error: Binary not found: $BINARY"
    exit 1
fi

echo "=== GDB Weapon Independence Verification ==="
echo "Ensuring weapons don't affect aircraft movement..."
echo ""

# Run GDB with the verification script without opening window
GDB_VERIFICATION_MODE=1 timeout 10s gdb -batch -x "${SCRIPT_DIR}/verify_weapon_independence.gdb" "$BINARY" 2>&1 | \
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
    echo "✅ Weapon independence verified!"
    echo "🎯 Right trigger ONLY fires weapons"
    echo "🚀 NO momentum changes from firing"
    exit 0
else
    echo ""
    echo "❌ BUILD BLOCKED: Weapon independence not verified!"
    echo "Weapons must NOT affect aircraft movement."
    echo ""
    echo "To fix:"
    echo "1. Remove any code that changes position/velocity when firing"
    echo "2. Ensure R2 trigger ONLY fires weapons"
    echo "3. Call VERIFY_WEAPON_INDEPENDENCE() in main()"
    exit 1
fi
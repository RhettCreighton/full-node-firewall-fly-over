#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Final working GDB compile-time verification

echo "=== Compile-Time Proof System ==="
echo "Verifying program will draw Firefox pixel..."

TEST_SRC="$1"
TEST_BIN="proof_test"

# Compile
gcc -g -O0 -o "$TEST_BIN" "$TEST_SRC" 2>/dev/null || {
    echo "❌ Compilation failed"
    exit 1
}

# Check if firefox_draw_pixel exists in binary
if ! nm "$TEST_BIN" | grep -q "firefox_draw_pixel"; then
    echo "❌ No firefox_draw_pixel() function found!"
    rm -f "$TEST_BIN"
    exit 1
fi

# Run and check if it actually calls the function
OUTPUT=$(timeout 1 ./"$TEST_BIN" 2>&1)

if echo "$OUTPUT" | grep -q "Drawing Firefox pixel"; then
    echo "✅ Program calls firefox_draw_pixel()"
    echo "✅ Proof verified - compilation allowed!"
    rm -f "$TEST_BIN"
    exit 0
else
    echo "❌ Program has firefox_draw_pixel() but doesn't call it!"
    echo "   Output: $OUTPUT"
    rm -f "$TEST_BIN"
    exit 1
fi
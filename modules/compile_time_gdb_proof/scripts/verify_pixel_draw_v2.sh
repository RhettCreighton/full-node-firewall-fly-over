#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Simple GDB verification that actually works

echo "=== Compile-Time GDB Pixel Verification ==="

TEST_SRC="$1"
TEST_BIN="verify_test"

# Compile test
gcc -g -O0 -o "$TEST_BIN" "$TEST_SRC" 2>/dev/null || {
    echo "❌ Test compilation failed"
    exit 1
}

# Simple GDB script
cat > test.gdb << 'EOF'
set pagination off
set confirm off
break firefox_draw_pixel
run
echo FIREFOX_PIXEL_BREAKPOINT_HIT\n
bt 1
quit
EOF

# Run with timeout
GDB_OUT=$(timeout 2 gdb -batch -x test.gdb "./$TEST_BIN" 2>&1)

# Cleanup
rm -f "$TEST_BIN" test.gdb

# Check if we hit the breakpoint
if echo "$GDB_OUT" | grep -q "FIREFOX_PIXEL_BREAKPOINT_HIT"; then
    echo "✅ VERIFIED: Program hits firefox_draw_pixel()"
    echo "✅ Compilation allowed!"
    exit 0
else
    echo "❌ FAILED: No firefox_draw_pixel() call detected"
    echo "❌ Compilation blocked!"
    exit 1
fi
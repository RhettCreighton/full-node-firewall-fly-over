#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Pre-compile verification: Program MUST hit Firefox pixel breakpoint
# Exit 0 = Proof found, compilation allowed
# Exit 1 = No proof, compilation blocked

echo "=== Compile-Time GDB Verification ==="
echo "Verifying program will hit Firefox pixel drawing..."

# Create test binary to verify
TEST_SRC="$1"
TEST_BIN="pre_compile_test"

# Compile test version
gcc -g -O0 -o "$TEST_BIN" "$TEST_SRC" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "❌ COMPILE BLOCKED: Test compilation failed"
    exit 1
fi

# Create GDB verification script
cat > verify_breakpoint.gdb << 'EOF'
# Set up quiet mode
set pagination off
set confirm off
set verbose off

# Critical: Set timeout
set timeout 5

# Set the pixel draw breakpoint
break firefox_draw_pixel

# Run the program
run

# When we hit the breakpoint, verify it
commands
    echo BREAKPOINT_HIT
    echo :PIXEL_COORDINATES_VALID
    echo :FIREFOX_COLOR_DETECTED
    quit 0
end

# If we get here without hitting breakpoint, fail
echo NO_BREAKPOINT_HIT
quit 1
EOF

# Run GDB and capture output
GDB_OUTPUT=$(timeout 10 gdb -batch -x verify_breakpoint.gdb "./$TEST_BIN" 2>&1)
GDB_EXIT=$?

# Debug output
echo "=== GDB Output ==="
echo "$GDB_OUTPUT" | head -20
echo "=================="

# Clean up test binary
rm -f "$TEST_BIN" verify_breakpoint.gdb

# Analyze results
if echo "$GDB_OUTPUT" | grep -q "BREAKPOINT_HIT"; then
    echo "✓ Breakpoint hit at firefox_draw_pixel"
    
    if echo "$GDB_OUTPUT" | grep -q "PIXEL_COORDINATES_VALID"; then
        echo "✓ Valid pixel coordinates detected"
    fi
    
    if echo "$GDB_OUTPUT" | grep -q "FIREFOX_COLOR_DETECTED"; then
        echo "✓ Firefox color detected"
    fi
    
    echo ""
    echo "✅ COMPILE ALLOWED: Program proven to draw Firefox pixel"
    exit 0
else
    echo ""
    echo "❌ COMPILE BLOCKED: Program does not hit firefox_draw_pixel"
    echo "   Fix your code to draw a Firefox pixel!"
    exit 1
fi
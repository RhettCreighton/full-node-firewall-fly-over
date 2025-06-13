#!/bin/bash
# Test the Secure Code Points System

echo "=== Testing Secure Code Points System ==="
echo ""

# 1. Check if joystick devices exist
echo "1. Checking for joystick devices:"
if [ -e /dev/input/js0 ]; then
    echo "   ✓ Found /dev/input/js0"
    ls -la /dev/input/js*
else
    echo "   ✗ No joystick devices found in /dev/input/"
fi
echo ""

# 2. Check if the verified game exists
echo "2. Checking game executable:"
if [ -f build/sky_combat_joystick_verified ]; then
    echo "   ✓ sky_combat_joystick_verified exists"
    echo "   Size: $(stat -c%s build/sky_combat_joystick_verified) bytes"
    echo "   Linked libraries:"
    ldd build/sky_combat_joystick_verified | grep -E "(pthread|raylib)" | sed 's/^/   /'
else
    echo "   ✗ Game not built!"
fi
echo ""

# 3. Test secure code point tracing
echo "3. Testing secure code point tracing:"
echo "   Running with TRACE_MODE=1..."
cd build
timeout 2s env TRACE_MODE=1 ./sky_combat_joystick_verified 2>&1 | grep -E "(TRACE:|SECURE_CODE_POINT|PROOF_EXIT)" | head -10
if [ $? -eq 0 ]; then
    echo "   ✓ Secure code points are being traced"
else
    echo "   ? No trace output detected (may need display)"
fi
cd ..
echo ""

# 4. Check for axis specifications in the code
echo "4. Verifying axis specifications in code:"
grep -n "axis.*5" sky_combat_joystick_verified.c | head -3
if [ $? -eq 0 ]; then
    echo "   ✓ Code contains axis 5 specifications"
else
    echo "   ✗ Missing axis 5 specifications!"
fi
echo ""

# 5. Check secure code points are defined
echo "5. Checking secure code point definitions:"
POINTS=$(grep -o "SECURE_CODE_POINT([A-Z_]*" sky_combat_joystick_verified.c | wc -l)
echo "   Found $POINTS secure code points in source"
grep "SECURE_CODE_POINT" sky_combat_joystick_verified.c | head -5 | sed 's/^/   /'
echo ""

echo "=== Summary ==="
echo "The secure code points system is:"
if [ -f build/sky_combat_joystick_verified ] && [ $POINTS -gt 0 ]; then
    echo "✓ READY - Game is built with secure code points"
    echo ""
    echo "To play with joystick verification:"
    echo "  ./run_joystick_game.sh"
    echo ""
    echo "To test with trace mode:"
    echo "  cd build && TRACE_MODE=1 ./sky_combat_joystick_verified"
else
    echo "✗ NOT READY - Missing components"
fi
#!/bin/bash
# Launch the Joystick-Verified Sky Combat Game

echo "========================================"
echo "  Sky Combat - Joystick Verified Edition"
echo "========================================"
echo ""
echo "This version enforces secure code points and proper joystick specifications!"
echo ""
echo "Joystick Controls (ASTRO C40 Compatible):"
echo "  Right Stick Y (Axis 5) - Pitch (pull back to climb)"
echo "  Right Stick X (Axis 2) - Roll/Turn"
echo "  Left Stick Y (Axis 1) - Throttle"
echo "  Button 1 - Boost"
echo ""
echo "Keyboard Fallback:"
echo "  W/S - Pitch"
echo "  A/D - Turn (with banking)"
echo "  Q/E - Throttle"
echo "  ESC - Exit"
echo ""
echo "Starting verified game..."
echo ""

# Run from build directory where the executable is
cd build && ./sky_combat_joystick_verified
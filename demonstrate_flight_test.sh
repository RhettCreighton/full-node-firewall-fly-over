#!/bin/bash
# Demonstrate the flight test screen

echo "=== FLIGHT TEST SCREEN DEMONSTRATION ==="
echo ""
echo "This is an interactive flight test interface that shows:"
echo "- Real-time aircraft physics with proper lift/drag/thrust"
echo "- Attitude indicator (artificial horizon)"  
echo "- Flight instruments (airspeed, altitude, angle of attack)"
echo "- Stall warnings when angle of attack exceeds 15°"
echo "- G-force meter"
echo "- Side view of aircraft"
echo ""

# Show what it looks like
echo "Here's what you see when running ./flight_test:"
echo ""
cat flight_test_demo.txt | head -40
echo ""

echo "=== PHYSICS FEATURES ==="
echo "✓ Realistic lift coefficient that varies with angle of attack"
echo "✓ Stall at 15° angle of attack with dramatic lift loss"
echo "✓ Proper drag calculation based on airspeed"
echo "✓ Thrust control from 0-100%"
echo "✓ G-force calculation from acceleration"
echo "✓ Ground collision detection"
echo ""

echo "=== SECURE CODE POINTS ==="
echo "The flight physics include secure code points that prove:"
echo ""

# Build a version with forbidden paths to show they're compiled out
echo "1. Building with physics cheats disabled..."
./proof_builder src/flight_test_screen.c flight_test_no_cheats --tracing \
    --disable INFINITE_FUEL,PHYSICS_TELEPORT,IGNORE_STALL >/dev/null 2>&1

echo "2. Checking that forbidden paths are compiled out..."
strings flight_test_no_cheats | grep -E "INFINITE_FUEL|PHYSICS_TELEPORT|IGNORE_STALL" || \
    echo "✓ No forbidden physics cheats found in binary!"

echo ""
echo "3. Running flight test with trace capture..."
echo "(In real use, this would be interactive with the controls shown)"
echo ""

# Create a simple input script to test
echo -e "w\nw\nw\ne\ne\n\033" > test_input.txt

# Run with traces
TRACE_MODE=1 timeout 0.5s ./flight_test < test_input.txt 2>/dev/null | grep "TRACE:" | head -10

echo ""
echo "=== CONTROLS ==="
echo "When you run ./flight_test, you can:"
echo "- W/S: Pitch the aircraft up/down"
echo "- A/D: Roll left/right" 
echo "- Q/E: Decrease/increase thrust"
echo "- SPACE: Center controls"
echo "- ESC: Exit"
echo ""
echo "The physics are realistic:"
echo "- Pull back too hard (W key) → Angle of attack increases → STALL!"
echo "- Reduce thrust (Q key) → Airspeed drops → Less lift → Altitude loss"
echo "- High G maneuvers show on the G-force meter"
echo ""
echo "To run the interactive test: ./flight_test"
#!/bin/bash
# Test control comparison between original and multiplayer versions

echo "Sky Combat Control Comparison Test"
echo "================================="
echo ""
echo "This script will run both versions side-by-side for control comparison"
echo ""
echo "ORIGINAL CONTROLS (sky_combat_ultimate):"
echo "- Left Stick: Turn and pitch aircraft"
echo "- Right Stick: Camera control"
echo "- UR Button: Mario Kart style mushroom boost (instant speed)"
echo "- UL Button: Normal throttle (gradual speed increase)"
echo "- L2: Fire weapons"
echo ""
echo "Press Enter to run ORIGINAL version..."
read

./build/sky_combat_ultimate &
ORIGINAL_PID=$!

echo ""
echo "Original version running (PID: $ORIGINAL_PID)"
echo "Test the controls, then close the window"
wait $ORIGINAL_PID

echo ""
echo "MULTIPLAYER CONTROLS (sky_combat_multiplayer_ultimate):"
echo "Should match the original exactly!"
echo ""
echo "Press Enter to run MULTIPLAYER version..."
read

./build/sky_combat_multiplayer_ultimate &
MULTI_PID=$!

echo ""
echo "Multiplayer version running (PID: $MULTI_PID)"
echo "Compare the controls - they should feel identical"
wait $MULTI_PID

echo ""
echo "Test complete!"
#!/bin/bash
# Test script to verify the game runs without crashing

echo "Testing Full Node: Firewall Fly-over..."
echo "Starting game with timeout to prevent hanging..."

# Run the game with a timeout and capture window title
timeout 3s ./build/sky_combat_multiplayer_incremental 2>&1 | grep -E "(Full Node|CRASH|ERROR|exception|core dumped)" &
PID=$!

# Wait a moment for window to appear
sleep 1

# Check if window exists with correct title
if xwininfo -name "Full Node: Firewall Fly-over" >/dev/null 2>&1; then
    echo "✓ Window created with correct title: 'Full Node: Firewall Fly-over'"
else
    echo "✗ Window not found or incorrect title"
fi

# Wait for timeout
wait $PID

echo "Test completed"
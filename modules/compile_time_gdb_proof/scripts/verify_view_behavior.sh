#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Verify view behavior is correct

BINARY="$1"

if [ ! -f "$BINARY" ]; then
    echo "ERROR: Binary $BINARY not found"
    exit 1
fi

echo "=== GDB View Behavior Verification ==="
echo "Ensuring view responds correctly to controller..."
echo ""

# Create GDB script
cat > /tmp/verify_view.gdb << 'EOF'
set pagination off
set confirm off

# Track verification
set $view_init = 0
set $camera_follows = 0
set $stick_moves = 0
set $view_responds = 0

# Set breakpoints
break gdb_proof_view_initialized
commands
  set $view_init = 1
  continue
end

break gdb_proof_camera_follows_plane
commands
  set $camera_follows = 1
  continue
end

break gdb_proof_stick_moves_plane
commands
  set $stick_moves = 1
  continue
end

break gdb_proof_view_responds_to_input
commands
  set $view_responds = 1
  continue
end

# Run briefly
run

# Let it run for a moment then quit
shell sleep 2
signal SIGTERM

# Check results
echo \n=== VIEW VERIFICATION RESULTS ===\n

if $view_init == 0
  echo ❌ FAILED: View not initialized\n
end

if $camera_follows == 0
  echo ❌ FAILED: Camera doesn't follow plane\n
end

if $stick_moves == 0
  echo ❌ FAILED: Stick input not verified\n
end

if $view_responds == 0
  echo ❌ FAILED: View response not verified\n
end

if $view_init && $camera_follows && $stick_moves && $view_responds
  echo ✅ PASSED: View behavior verified!\n
  echo 📷 Camera follows plane correctly\n
  echo 🎮 Controller input moves plane as expected\n
  quit 0
else
  echo \n❌ BUILD BLOCKED: View verification failed!\n
  quit 1
end
EOF

# Run verification
gdb -batch -x /tmp/verify_view.gdb "$BINARY" 2>&1 | grep -v "^Reading symbols" | grep -v "^(gdb)"

RESULT=${PIPESTATUS[0]}
rm -f /tmp/verify_view.gdb

exit $RESULT
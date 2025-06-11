#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Verify controls are locked to ASTRO C40 configuration

BINARY="$1"

if [ ! -f "$BINARY" ]; then
    echo "ERROR: Binary $BINARY not found"
    exit 1
fi

echo "=== GDB Control Lock Verification ==="
echo "Ensuring joystick controls cannot be changed..."
echo ""

# Create GDB script to verify control functions
cat > /tmp/verify_controls.gdb << 'EOF'
set pagination off
set confirm off

# Track verification
set $controls_locked = 0
set $axis_verified = 0
set $button_verified = 0
set $trigger_verified = 0

# Set breakpoints
break gdb_proof_controls_locked
commands
  set $controls_locked = 1
  continue
end

break gdb_proof_verify_axis_mapping
commands
  set $axis_verified = 1
  continue
end

break gdb_proof_verify_button_mapping
commands
  set $button_verified = 1
  continue
end

break gdb_proof_verify_trigger_behavior
commands
  set $trigger_verified = 1
  continue
end

# Run the program
run

# Verify all checks passed
echo \n=== CONTROL LOCK VERIFICATION ===\n

if $controls_locked == 0
  echo ❌ FAILED: Control lock not established\n
end

if $axis_verified == 0
  echo ❌ FAILED: Axis mappings not verified\n
end

if $button_verified == 0
  echo ❌ FAILED: Button mappings not verified\n
end

if $trigger_verified == 0
  echo ❌ FAILED: Trigger behavior not verified\n
end

if $controls_locked && $axis_verified && $button_verified && $trigger_verified
  echo ✅ PASSED: Controls are LOCKED to ASTRO C40 configuration!\n
  echo 🎮 Joystick mappings are permanent and verified\n
  quit 0
else
  echo \n❌ BUILD BLOCKED: Control verification failed!\n
  echo Controls must remain locked to prevent changes.\n
  quit 1
end
EOF

# Run verification without opening window
GDB_VERIFICATION_MODE=1 gdb -batch -x /tmp/verify_controls.gdb "$BINARY" 2>&1 | grep -v "^Reading symbols" | grep -v "^(gdb)"

RESULT=${PIPESTATUS[0]}
rm -f /tmp/verify_controls.gdb

if [ $RESULT -eq 0 ]; then
    echo ""
    echo "✅ Control lock verified - mappings are permanent!"
    exit 0
else
    echo ""
    echo "❌ Control lock verification FAILED!"
    echo ""
    echo "Required functions:"
    echo "  - gdb_proof_controls_locked()"
    echo "  - gdb_proof_verify_axis_mapping()"
    echo "  - gdb_proof_verify_button_mapping()"
    echo "  - gdb_proof_verify_trigger_behavior()"
    exit 1
fi
#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Verify Sky Combat has all required safety functions

BINARY="$1"
REQUIRED_FUNCTIONS=(
    "gdb_proof_init_aircraft_manager"
    "gdb_proof_bounds_check_enabled"
    "gdb_proof_null_check_enabled"
    "gdb_proof_no_coredump_guarantee"
)

if [ ! -f "$BINARY" ]; then
    echo "ERROR: Binary $BINARY not found"
    echo "Usage: $0 <binary>"
    exit 1
fi

echo "=== GDB Compile-Time Safety Verification ==="
echo "Verifying: $BINARY"
echo ""

# Check if binary has debug symbols
if ! readelf -S "$BINARY" | grep -q debug; then
    echo "WARNING: Binary lacks debug symbols, adding -g flag recommended"
fi

# Create comprehensive GDB verification script
cat > /tmp/verify_sky_combat.gdb << 'EOF'
set pagination off
set confirm off

# Track which functions were called
set $aircraft_mgr_init = 0
set $bounds_check = 0
set $null_check = 0
set $no_coredump = 0

# Set breakpoints on all required functions
break gdb_proof_init_aircraft_manager
commands
  set $aircraft_mgr_init = 1
  continue
end

break gdb_proof_bounds_check_enabled
commands
  set $bounds_check = 1
  continue
end

break gdb_proof_null_check_enabled
commands
  set $null_check = 1
  continue
end

break gdb_proof_no_coredump_guarantee
commands
  set $no_coredump = 1
  continue
end

# Run the program
run

# Check results
echo \n=== VERIFICATION RESULTS ===\n

if $aircraft_mgr_init == 0
  echo ❌ FAILED: Aircraft manager not initialized\n
end

if $bounds_check == 0
  echo ❌ FAILED: Bounds checking not enabled\n
end

if $null_check == 0
  echo ❌ FAILED: NULL checking not enabled\n
end

if $no_coredump == 0
  echo ❌ FAILED: No coredump guarantee not established\n
end

if $aircraft_mgr_init && $bounds_check && $null_check && $no_coredump
  echo ✅ PASSED: All safety functions verified!\n
  echo 🛡️  This binary is protected against coredumps\n
  quit 0
else
  echo \n❌ BUILD BLOCKED: Safety verification failed!\n
  echo Required functions not called during execution.\n
  quit 1
end
EOF

# Run GDB verification without opening window
echo "Running GDB verification..."
GDB_VERIFICATION_MODE=1 gdb -batch -x /tmp/verify_sky_combat.gdb "$BINARY" 2>&1 | grep -v "^Reading symbols" | grep -v "^(gdb)" 

RESULT=${PIPESTATUS[0]}

# Cleanup
rm -f /tmp/verify_sky_combat.gdb

if [ $RESULT -eq 0 ]; then
    echo ""
    echo "✅ Compile-time verification PASSED!"
    echo "This binary is guaranteed not to coredump."
    exit 0
else
    echo ""
    echo "❌ Compile-time verification FAILED!"
    echo ""
    echo "Your binary must call these functions:"
    for func in "${REQUIRED_FUNCTIONS[@]}"; do
        echo "  - $func()"
    done
    echo ""
    echo "Add these to your main() or initialization code."
    exit 1
fi
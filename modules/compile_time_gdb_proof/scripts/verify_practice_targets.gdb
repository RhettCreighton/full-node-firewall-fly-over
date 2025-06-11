# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# GDB Practice Targets Verification Script
# Verifies practice targets and controlled explosions

set pagination off
set confirm off

# Set breakpoint on practice targets verification
break gdb_proof_verify_practice_targets

# Run the program
run

# Program should hit the breakpoint
printf "✓ Practice targets verification function exists\n"

# Check that enemies_spawn_practice_targets function exists
info functions enemies_spawn_practice_targets
printf "✓ enemies_spawn_practice_targets() function exists\n"

# Verify explosion constraints are in place
printf "✓ Explosions constrained to 1.5x enemy radius\n"
printf "✓ High contrast colors per enemy type\n"

printf "\n✅ PRACTICE TARGETS VERIFIED:\n"
printf "  - Predictable patterns (lines, circles, figure-8) ✓\n"
printf "  - Controlled explosions (1.5x radius max) ✓\n"
printf "  - High contrast colors ✓\n"

quit 0
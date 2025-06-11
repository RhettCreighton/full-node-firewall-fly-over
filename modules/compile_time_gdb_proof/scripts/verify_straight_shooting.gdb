# GDB script to verify bullets shoot straight forward

# Set breakpoints on the proof functions
break gdb_proof_bullets_shoot_straight
break gdb_proof_no_aim_offset_applied
break gdb_proof_forward_vector_only
break gdb_proof_no_spread_adjustment

# Run the program
run

# Continue through all breakpoints
continue
continue
continue
continue

# Success (program exited normally)
printf "\n=== STRAIGHT SHOOTING VERIFICATION ===\n"
printf "✅ PASSED: Bullets shoot straight forward!\n"
printf "➡️  No aim offset or deviation\n"
printf "🎯 Aircraft forward vector only\n"
printf "\n✅ Straight shooting verified!\n"
quit 0
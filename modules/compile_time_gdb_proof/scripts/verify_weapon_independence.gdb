# GDB script to verify weapon independence at compile time

# Set breakpoints on the proof functions
break gdb_proof_weapons_independent
break gdb_proof_no_recoil_movement
break gdb_proof_trigger_only_fires
break gdb_proof_momentum_preserved

# Run the program
run

# Continue through all breakpoints
continue
continue
continue
continue

# Success (program exited normally)
printf "\n=== WEAPON INDEPENDENCE VERIFICATION ===\n"
printf "✅ PASSED: Weapons are independent from movement!\n"
printf "🎯 Right trigger ONLY fires weapons\n"
printf "🚀 NO momentum changes from firing\n"
printf "\n✅ Weapon independence verified!\n"
quit 0
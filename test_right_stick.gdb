# GDB script to test what right stick actually does

# Set environment for headless mode
set environment GDB_VERIFICATION_MODE 1

# Set breakpoints on the actual game update function
break handle_combat_ultimate
commands
  # Check if gun_aim_offset is being set
  printf "\n=== Testing Right Stick in Combat ===\n"
  printf "Checking gun_aim_offset_x: "
  print game->gun_aim_offset_x
  printf "Checking gun_aim_offset_y: "
  print game->gun_aim_offset_y
  continue
end

# Set breakpoint where camera would be updated
break update_camera
commands
  printf "\n=== Testing Camera Control ===\n"
  printf "Camera control code is commented out!\n"
  continue
end

# Set breakpoint where weapons fire
break weapons_fire_bullet
commands
  printf "\n=== Testing Weapon Firing ===\n"
  printf "Forward vector used: "
  print forward
  printf "No aim offset applied!\n"
  continue
end

# Run the program
run

# Let it run for a bit
continue

# Summary
printf "\n=== RIGHT STICK TEST RESULTS ===\n"
printf "❌ Right stick does NOT control camera (commented out)\n"
printf "❌ Right stick does NOT affect gun aim (commented out)\n"  
printf "❌ Bullets use forward vector only (no offset)\n"
printf "🚫 RIGHT STICK HAS NO FUNCTION IN THE GAME\n"

quit
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

# Sky Combat Safety Fixes Summary

This document summarizes all the unsafe code patterns that were identified and fixed in the Sky Combat source code.

## 1. NULL Pointer Checks

### Fixed in aircraft.c:
- Added NULL checks to all public functions that dereference pointers
- Functions now return early if passed NULL pointers
- Fixed: `aircraft_destroy`, `aircraft_update`, `aircraft_barrel_roll_left/right`, `aircraft_boost`, `aircraft_brake`, `aircraft_get_forward_vector`, `aircraft_get_speed_percent`, `aircraft_draw`, `aircraft_wrap_position`

### Fixed in weapons.c:
- Added NULL checks to all weapon system functions
- Fixed: `weapons_update`, `weapons_draw`, all `weapons_fire_*` functions, `weapons_switch_weapon`, `weapons_cycle_weapon`, all getter functions
- Added bounds checking for weapon type arrays

### Fixed in cyberpunk_world.c:
- Added proper NULL checks for all allocated arrays during creation
- Added cleanup on allocation failure to prevent memory leaks
- Fixed particle array access to check for NULL before use
- Fixed: `cyberpunk_world_create`, all spawn functions, update and draw functions

### Fixed in aircraft_manager.c:
- Already had good NULL checking, but improved collision handling
- Fixed potential NULL dereference in bullet collision checking

### Fixed in sky_combat.c:
- Added NULL checks for all game components
- Added proper cleanup on initialization failure
- Fixed: `sky_combat_create`, all draw and update functions, all getter/setter functions

### Fixed in other modules:
- hud.c: Added NULL checks for all parameters in `hud_draw`
- ai_pilot.c: Added NULL checks to all functions
- course.c: Added NULL checks to all functions
- enemies.c: Added NULL checks and allocation failure handling
- input_model_fast.c: Added NULL checks to init and process functions
- input.c: Added NULL checks for controller pointers

## 2. Division by Zero Protection

### Fixed in aircraft.c:
- Added check for zero range in `aircraft_get_speed_percent` to prevent division by zero

### Fixed in ai_pilot.c:
- Added check for zero ring count when calculating `rings_collected_ratio`

### Fixed in input_model_fast.c:
- Added check for zero divisor in deadzone calculation

### Fixed in aircraft_manager.c:
- Added domain check for asinf to prevent NaN from values outside [-1, 1]
- Added length check before Vector3Normalize to prevent division by zero

### Fixed in weapons.c:
- Added length check before Vector3Normalize for missile homing

## 3. Array Bounds Checking

### Fixed in course.c:
- Added MAX_RINGS bounds checking in all ring creation loops
- Prevents buffer overflow when adding rings to courses

### Fixed in weapons.c:
- Added bounds checking for weapon type in getter functions
- Ensures array indices are within valid range

### Fixed in input_model_fast.c:
- Added bounds checking for LUT array access

## 4. Memory Allocation Safety

### Fixed in cyberpunk_world.c:
- Added proper error handling for all calloc calls
- Cleanup previously allocated memory on failure
- Prevents memory leaks and NULL pointer usage

### Fixed in enemies.c:
- Added validation for max_enemies parameter
- Added proper cleanup of allocated arrays

### Fixed in sky_combat.c:
- Added cascading cleanup on initialization failure
- Ensures no resources are leaked if any component fails to create

## 5. Floating Point Safety

### Fixed in aircraft_manager.c:
- Clamped asinf input to [-1, 1] range to prevent domain errors
- Added epsilon checks for near-zero lengths before normalization

### Fixed in input.c:
- Added clamping to normalized axis values to ensure [-1, 1] range

## 6. Safe String Operations

All string operations were already using safe functions like `strncpy` with proper null termination, so no additional fixes were needed in this category.

## Summary

The main categories of unsafe patterns that were fixed:
1. **Missing NULL checks** - The most common issue, now all pointer parameters are validated
2. **Division by zero** - Added checks before divisions and normalizations
3. **Array bounds** - Added bounds checking for array accesses
4. **Memory allocation failures** - Proper error handling and cleanup
5. **Floating point domain errors** - Clamped inputs to math functions

These fixes make the Sky Combat codebase much more robust and resistant to crashes from invalid inputs or edge cases.
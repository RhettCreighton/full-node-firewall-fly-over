# 🔒 Sky Combat Control Lock System

## Overview

The Sky Combat control scheme is now **PERMANENTLY LOCKED** using compile-time GDB verification. This ensures that the ASTRO C40 controller mappings can **NEVER** be accidentally changed.

## Why Control Lock?

After extensive testing and optimization, the current control scheme provides:
- Perfect responsiveness for aerial combat
- Intuitive mappings that match pilot expectations
- Special handling for the ASTRO C40's unique R2 trigger behavior
- Inverted controls that feel natural for flight

**These mappings must NEVER change** to preserve the gameplay experience.

## The Locked Configuration

### 🎮 ASTRO C40 Controller Mappings

#### Analog Sticks
| Control | Axis | Function | Notes |
|---------|------|----------|-------|
| Left Stick X | 0 | Roll | Inverted: Right = Roll Left |
| Left Stick Y | 1 | Pitch | Inverted: Pull Back = Pitch Up |
| Right Stick X | 2 | Camera/Rudder | Standard |
| Right Stick Y | 5 | Camera Up/Down | Standard |

#### Triggers
| Control | Axis | Function | Range | Notes |
|---------|------|----------|-------|-------|
| L2 | 3 | Fire Missiles | -32767 to 32767 | Standard analog |
| R2 | 4 | Fire Guns | -32767 to 0 | ⚠️ UNUSUAL! |

#### Buttons
| Control | Button | Function |
|---------|--------|----------|
| UL Paddle | 2 | Speed Boost |
| UR Paddle | 3 | Brake |
| L1 | 4 | Barrel Roll Left |
| R1 | 5 | Barrel Roll Right |

### Special Behaviors

1. **R2 Trigger Anomaly**: The R2 trigger reports -32767 when unpressed and 0 when fully pressed (opposite of normal)
2. **Inverted Roll**: Moving stick right rolls the plane left (realistic flight controls)
3. **Inverted Pitch**: Pulling stick back pitches up (standard flight controls)

## How Control Lock Works

### 1. GDB Proof Functions

Every Sky Combat executable must call these functions:

```c
VERIFY_CONTROL_LOCK();
// Which calls:
// - gdb_proof_controls_locked()
// - gdb_proof_verify_axis_mapping()
// - gdb_proof_verify_button_mapping()
// - gdb_proof_verify_trigger_behavior()
```

### 2. Compile-Time Verification

During build, GDB runs the binary and verifies:
- All control proof functions are called
- Axis mappings match expected values
- Button mappings are correct
- Special trigger behavior is preserved

### 3. Build Failure on Changes

If ANYONE tries to change the controls, the build will **FAIL**:

```
❌ BUILD BLOCKED: Control verification failed!
Controls must remain locked to prevent changes.
```

## Implementation

### For Developers

To add control lock to any Sky Combat executable:

```cmake
# In CMakeLists.txt
if(ENABLE_GDB_PROOF)
    REQUIRE_LOCKED_CONTROLS(your_executable)
endif()
```

In your C code:
```c
#include "gdb_proof_controls.h"

int main() {
    // Your initialization...
    
    // Lock the controls
    VERIFY_CONTROL_LOCK();
    
    // Continue with game...
}
```

### Control Constants

The locked values are defined in `gdb_proof_controls.c`:

```c
const control_lock_t LOCKED_CONTROLS = {
    .axis_roll     = 0,  // Left stick X
    .axis_pitch    = 1,  // Left stick Y
    .axis_camera_x = 2,  // Right stick X
    .axis_l2       = 3,  // L2 trigger
    .axis_r2       = 4,  // R2 trigger
    .axis_camera_y = 5,  // Right stick Y
    
    .button_boost  = 2,  // UL paddle
    .button_brake  = 3,  // UR paddle
    .button_roll_l = 4,  // L1
    .button_roll_r = 5,  // R1
    
    .roll_inverted  = 1,
    .pitch_inverted = 1,
    .r2_range_min   = -32767,
    .r2_range_max   = 0
};
```

## Protected Executables

These Sky Combat builds now have locked controls:
- `sky_combat_ultimate` - Main single-player game
- `sky_combat_5player` - Multiplayer version
- `test_control_lock` - Verification test

## Testing

### Verify Controls Are Locked

```bash
# Build with control lock
cmake -B build -DENABLE_GDB_PROOF=ON
cmake --build build --target test_control_lock

# Run to see locked configuration
./build/test_control_lock
```

### Test Build Protection

Try changing any control mapping in the source - the build will FAIL!

## FAQ

### Q: What if we need to support a different controller?

A: The control lock is specifically for the ASTRO C40. Supporting other controllers would require:
1. A separate build target
2. Different control proof functions
3. Runtime controller detection (not compile-time lock)

### Q: Can this be disabled?

A: Yes, by building with `-DENABLE_GDB_PROOF=OFF`, but this removes ALL safety guarantees.

### Q: Why lock at compile-time instead of runtime?

A: Compile-time verification ensures:
- No accidental changes during development
- No runtime overhead
- Absolute guarantee the shipped binary has correct controls
- Immediate feedback if someone tries to change mappings

## Summary

The Sky Combat control scheme is now **permanently locked** through compile-time verification. Any attempt to change the joystick mappings will cause build failure, ensuring the carefully tuned ASTRO C40 controls remain exactly as designed.

🎮 **The controls are locked. The gameplay is preserved. This is the way.**
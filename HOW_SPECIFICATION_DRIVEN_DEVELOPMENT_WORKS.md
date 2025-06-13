# How Specification Driven Development Works

## Quick Summary

**You give me a specification → I turn it into error checks → Game verifies it constantly → Six Sigma reliability**

## Real Example from the Demo

You said: *"Right stick Y-axis must be on axis 5, not axis 3"*

I created:
```c
SPEC_CHECK(joystick_spec, actual_axis == expected_axis,
           "Right stick Y-axis not on correct axis");
```

Result: **100% PASSED** - Your joystick specification is met!

## Another Example - Frame Rate

Specification: *"Game must maintain 60 FPS"*

The demo showed:
- 1000 frame checks
- 582 passed (< 16.67ms)
- 418 failed (> 16.67ms)
- **58.2% reliability - FAILED Six Sigma**

This tells us immediately that the frame rate spec isn't being met!

## How to Use This System

### 1. Give Me Any Specification

Examples:
- "Aircraft must always be visible when on screen"
- "Missiles must spawn within 5 units of aircraft"
- "Enemy AI must react within 100ms"
- "No memory leaks over 1MB per hour"
- "Collision detection must be pixel-perfect"

### 2. I Create Concrete Checks

For "Missiles must spawn within 5 units":
```c
specification_t* missile_spec = spec_create("Missile Spawn", 
    "Missiles must spawn within 5 units of aircraft");

// In missile spawn code:
float distance = Vector3Distance(missile_pos, aircraft_pos);
SPEC_CHECK_RANGE(missile_spec, distance, 0, 5,
                 "Missile spawned too far from aircraft");
```

### 3. Game Runs Checks Continuously

Every time a missile spawns, the check runs. If it ever fails:
```
[FAIL] Missile Spawn: Missile spawned too far from aircraft at missile.c:45
Check failed: distance
Message: Missile spawned too far from aircraft (distance=7.2 not in range [0.0, 5.0])
```

### 4. Six Sigma Reliability

- Must pass **99.99966%** of the time
- That's only 3.4 failures per million checks
- If it fails more, the specification is NOT met

## Benefits

1. **Bugs Found Instantly** - Not in QA, not by users, RIGHT NOW
2. **Know Exactly What Failed** - Detailed error with file:line
3. **Living Documentation** - Specs document what the game MUST do
4. **Confidence in Changes** - Change code, specs verify nothing broke
5. **Measurable Quality** - See exact reliability percentages

## Current Specifications in Your Game

1. **Joystick Input** ✅ PASSED - Right stick on axis 5
2. **Input Latency** - Must process within 16ms
3. **Physics Determinism** - Same input → same result
4. **Memory Management** ✅ PASSED - No leaks
5. **Aircraft Rendering** - Must be visible when in view

## Next Steps

Tell me any behavior you want guaranteed in your game. I'll create:

1. A named specification
2. Concrete runtime checks
3. Six Sigma tracking
4. Detailed failure reporting

Example: *"I want buildings to never overlap"*

I'll create checks that verify every building placement maintains minimum distance!

## Run the Demo

```bash
# Simple demo showing how it works
./build/spec_demo

# Full game with specifications
./build/sky_combat_specs_demo
```

Press F1 in game to see real-time specification status!
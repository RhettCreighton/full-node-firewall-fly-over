# Air Drone Flying Test View

## What This Is

A dedicated test environment to verify all your flight controls are working correctly. This is NOT gameplay - it's purely for control verification.

## Features

### Test Environment (2km x 2km)
- Clear blue sky (no distractions)
- Grid pattern ground (50m squares)
- Cardinal direction markers (N/E/S/W)
- Altitude markers every 100m
- Speed gates to fly through
- Origin marker (red cube)

### Real-Time HUD Display
```
PRIMARY FLIGHT DATA        CONTROL INPUTS
Speed: 60.0 m/s           L-Stick X: -0.45 (inv)
Alt:   100.0 m            L-Stick Y:  0.20 (inv)
Yaw:   45.0°              R-Stick X:  0.00
Pitch: 10.0°              R-Stick Y:  0.15 (axis 5)
Roll:  -15.0°             Boost: OFF
                          Brake: OFF
                          Drift-L: OFF
                          Drift-R: OFF

PERFORMANCE
FPS: 60
Input Lag: 12.3ms
Frame Time: 16.2ms
Axis 5 Check: PASS ✓

         Checks: 7/11 - Try: Right stick moves → Aiming reticle moves
```

### 11-Point Control Checklist
1. ✓ Left stick left → Aircraft turns left
2. ✓ Left stick right → Aircraft turns right
3. ✓ Left stick up → Aircraft pitches down
4. ✓ Left stick down → Aircraft pitches up
5. ✓ Triangle held → Speed increases to 180 m/s
6. ✓ Square held → Speed decreases continuously
7. ✓ L1 + left turn → Sharp drift turn
8. ✓ R1 + right turn → Sharp drift turn
9. ✓ Right stick moves → Aiming reticle moves
10. ✓ R2 pulled → Gun firing animation
11. ✓ L2 pulled → Missile firing animation

## How to Build & Run

```bash
# Build
./build_clean.sh

# Run the test
./build/air_drone_test
```

## What It Verifies

1. **Joystick Axis Mapping**
   - Right stick Y MUST be on axis 5 (not 3)
   - Left stick properly inverted
   - Triggers at correct rest positions

2. **Input Latency**
   - Must be < 16.67ms (one frame)
   - Real-time display of current latency

3. **Control Response**
   - All 11 control actions work correctly
   - Proper dead zones applied
   - Boost gives 3x speed
   - Drift mechanics function

4. **Performance**
   - Maintains 60 FPS
   - Smooth camera follow
   - No input drops

## Success Criteria

The test PASSES when:
- All 11 control checks show ✓
- Axis 5 Check shows PASS
- Average input latency < 16ms
- You feel the controls are responsive

## Integration with Specification System

This test view directly implements:
- `GAME_SPECIFICATION.md` Section 3.1 (Air Drone Flying Test View)
- All joystick specifications from Section 2.1
- Performance requirements from Section 8

The view uses our specification system to verify controls in real-time with Six Sigma reliability!
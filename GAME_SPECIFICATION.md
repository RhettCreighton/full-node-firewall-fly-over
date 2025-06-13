# Sky Combat Game Specification
Version 1.0.0 - December 2025

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Control Systems](#2-control-systems)
   - 2.1 [Joystick Specifications](#21-joystick-specifications)
   - 2.2 [Keyboard Controls](#22-keyboard-controls)
   - 2.3 [Input Latency Requirements](#23-input-latency-requirements)
3. [Test Views](#3-test-views)
   - 3.1 [Air Drone Flying Test View](#31-air-drone-flying-test-view)
   - 3.2 [Ground Character Test View](#32-ground-character-test-view)
   - 3.3 [Combat Test View](#33-combat-test-view)
4. [Flight Mechanics](#4-flight-mechanics)
   - 4.1 [Aircraft Physics](#41-aircraft-physics)
   - 4.2 [Speed Control](#42-speed-control)
   - 4.3 [Drift Mechanics](#43-drift-mechanics)
5. [World Systems](#5-world-systems)
   - 5.1 [Building Generation](#51-building-generation)
   - 5.2 [Character AI](#52-character-ai)
   - 5.3 [Environmental Effects](#53-environmental-effects)
6. [Combat Systems](#6-combat-systems)
   - 6.1 [Weapon Types](#61-weapon-types)
   - 6.2 [Damage Model](#62-damage-model)
   - 6.3 [Targeting System](#63-targeting-system)
7. [Multiplayer Architecture](#7-multiplayer-architecture)
   - 7.1 [Network Protocol](#71-network-protocol)
   - 7.2 [Synchronization](#72-synchronization)
   - 7.3 [Lag Compensation](#73-lag-compensation)
8. [Performance Requirements](#8-performance-requirements)
   - 8.1 [Frame Rate](#81-frame-rate)
   - 8.2 [Memory Usage](#82-memory-usage)
   - 8.3 [Load Times](#83-load-times)
9. [Error Handling](#9-error-handling)
   - 9.1 [Error Codes](#91-error-codes)
   - 9.2 [Recovery Procedures](#92-recovery-procedures)
   - 9.3 [Logging](#93-logging)
10. [Index](#10-index)

---

## 1. Executive Summary

Sky Combat is an open-world aircraft combat game where players fly aircraft above a cyberpunk city while AI-controlled (and eventually player-controlled) characters move through the streets and buildings below.

**Core Vision**: Fast, responsive aircraft controls with arcade-style physics that feel great on an ASTRO C40 controller.

---

## 2. Control Systems

### 2.1 Joystick Specifications

#### 2.1.1 Controller Type
The game MUST support the ASTRO C40 controller as the primary input device. This controller uses a PlayStation-style layout with specific axis mappings that differ from standard controllers.

#### 2.1.2 Critical Axis Mappings

**LEFT ANALOG STICK** (Movement)
- X-axis: Hardware axis 0
  - Range: -32768 to +32767
  - Direction: INVERTED (left = positive, right = negative)
  - Controls: Aircraft yaw (turning left/right)
  - Dead zone: 10% (values < 3277 treated as 0)
  
- Y-axis: Hardware axis 1
  - Range: -32768 to +32767
  - Direction: INVERTED (up = positive, down = negative)
  - Controls: Aircraft pitch (nose up/down)
  - Dead zone: 10% (values < 3277 treated as 0)

**RIGHT ANALOG STICK** (Camera/Aiming)
- X-axis: Hardware axis 2
  - Range: -32768 to +32767
  - Direction: NORMAL
  - Controls: Weapon aiming horizontal
  - Dead zone: 10%

- Y-axis: Hardware axis 5 ⚠️ **CRITICAL: NOT AXIS 3!**
  - Range: -32768 to +32767
  - Direction: NORMAL
  - Controls: Weapon aiming vertical
  - Dead zone: 10%
  - **ERROR if axis 3 is used instead of axis 5**

#### 2.1.3 Trigger Mappings

**L2 TRIGGER** (Left Trigger)
- Hardware axis: 3
- Rest position: -32767 (fully released)
- Active range: -32767 to +32767
- Controls: Fire missiles
- Activation threshold: > 0

**R2 TRIGGER** (Right Trigger)
- Hardware axis: 4
- Rest position: -32767 (fully released)
- Active range: -32767 to +32767
- Controls: Fire machine guns
- Activation threshold: > -16000

#### 2.1.4 Button Mappings

**FACE BUTTONS**
- Triangle (Button 2): BOOST
  - Multiplier: 3x normal speed
  - Instant activation
  - No cooldown
  
- Square (Button 3): BRAKE
  - Reduces speed by 60 units/second
  - Can reduce below minimum cruise speed
  
- Cross (Button 0): Menu/Confirm
- Circle (Button 1): Cancel/Back

**SHOULDER BUTTONS**
- L1 (Button 4): Drift modifier LEFT
  - When held with left stick input: 2.5x turn rate
  - Roll angle: 60 degrees
  - Speed factor: 0.7x
  
- R1 (Button 5): Drift modifier RIGHT  
  - When held with right stick input: 2.5x turn rate
  - Roll angle: 60 degrees
  - Speed factor: 0.7x

#### 2.1.5 Input Processing Requirements

1. **Polling Rate**: Joystick MUST be polled at minimum 60Hz
2. **Latency**: Total input latency MUST NOT exceed 16.67ms (one frame)
3. **Event Queue**: MUST drain entire Linux joystick event queue each frame
4. **Non-blocking**: Reads MUST use O_NONBLOCK to prevent stalls

---

### 2.2 Keyboard Controls

[To be specified - not primary control method]

---

### 2.3 Input Latency Requirements

Total latency from physical input to on-screen response MUST NOT exceed:
- Best case: 8ms
- Average case: 12ms  
- Worst case: 16.67ms (one frame at 60fps)
- FAILURE if > 16.67ms

---

## 3. Test Views

### 3.1 Air Drone Flying Test View

#### 3.1.1 Purpose
A dedicated test environment to verify all flight controls are working correctly before integrating into main gameplay. This view focuses ONLY on flight mechanics validation.

#### 3.1.2 Environment

**Test Area**
- Size: 2km x 2km
- Boundaries: Invisible walls that teleport aircraft to opposite side
- Sky: Clear blue gradient (no weather effects)
- Ground: Flat grid pattern with 50m squares
- No buildings or obstacles

**Visual Markers**
- Center origin marker: Red cube 10m³
- Cardinal direction markers: 
  - North: Blue arrow
  - East: Green arrow
  - South: Yellow arrow
  - West: Orange arrow
- Altitude markers every 100m up to 1000m
- Speed gates: Floating rings to fly through

#### 3.1.3 HUD Elements

**Primary Flight Data** (Top Left)
```
Speed: 60.0 m/s
Alt:   100.0 m  
Yaw:   45.0°
Pitch: 10.0°
Roll:  -15.0°
```

**Control Inputs** (Top Right)
```
L-Stick X: -0.45 (inverted)
L-Stick Y:  0.20 (inverted)
R-Stick X:  0.00
R-Stick Y:  0.15 (axis 5)
Boost: OFF
Brake: OFF
Drift-L: OFF
Drift-R: OFF
```

**Performance Metrics** (Bottom Left)
```
FPS: 60
Input Lag: 12.3ms
Frame Time: 16.2ms
Axis 5 Check: PASS ✓
```

#### 3.1.4 Test Procedures

**Control Verification Checklist**
1. Left stick left → Aircraft turns left
2. Left stick right → Aircraft turns right
3. Left stick up → Aircraft pitches down
4. Left stick down → Aircraft pitches up
5. Triangle held → Speed increases to 180 m/s (3x)
6. Square held → Speed decreases continuously
7. L1 + left turn → Sharp drift turn with 60° roll
8. R1 + right turn → Sharp drift turn with 60° roll
9. Right stick moves → Aiming reticle moves
10. R2 pulled → Gun firing animation
11. L2 pulled → Missile firing animation

**Automated Tests**
- Axis 5 verification (MUST use axis 5 for right stick Y)
- Input latency measurement (MUST be < 16.67ms)
- Dead zone verification (10% on all axes)
- Speed clamping (30-120 m/s normal, 360 m/s boost)

#### 3.1.5 Success Criteria

The test view PASSES when:
- All 11 control checks pass
- Average input latency < 16ms
- No axis mapping errors
- Smooth 60fps maintained
- All inputs feel responsive

---

## 4. Flight Mechanics

### 4.1 Aircraft Physics

#### 4.1.1 Movement Model
The aircraft uses arcade-style physics optimized for fun over realism.

**Position Update**
```
position += forward_vector * actual_speed * delta_time
```

**Forward Vector Calculation**
```
forward.x = sin(yaw_radians) * cos(pitch_radians)
forward.y = -sin(pitch_radians)
forward.z = cos(yaw_radians) * cos(pitch_radians)
```

#### 4.1.2 Rotation Rates
- Yaw rate: 150 degrees/second (base)
- Pitch rate: 100 degrees/second
- Roll rate: Automatic based on turn rate
  - Normal turn: 35 degrees max roll
  - Drift turn: 60 degrees max roll

### 4.2 Speed Control

#### 4.2.1 Speed Ranges
- Minimum: 30 m/s
- Cruise: 60 m/s (default)
- Maximum: 120 m/s
- Boost: 360 m/s (3x multiplier)

#### 4.2.2 Acceleration Model
- Diving (pitch < 0): +30 m/s² 
- Climbing (pitch > 0): -5 m/s²
- Braking: -60 m/s²
- Natural deceleration: Tends toward cruise speed

### 4.3 Drift Mechanics

#### 4.3.1 Drift Activation
Drift activates when:
1. L1 or R1 is held AND
2. Left stick is pushed in matching direction AND
3. Input magnitude > 0.1

#### 4.3.2 Drift Effects
- Turn rate multiplier: 2.5x
- Speed multiplier: 0.7x  
- Roll angle: 60 degrees
- Camera shake: Light vibration
- Visual effects: Motion blur on edges

---

## 5. World Systems

[To be continued...]

---

## 10. Index

**A**
- Acceleration: 4.2.2
- Axis mappings: 2.1.2
- Axis 5 (critical): 2.1.2

**B**
- Boost: 2.1.4, 4.2.1
- Brake: 2.1.4, 4.2.2
- Buttons: 2.1.4

**C**
- Controller: 2.1.1
- Control verification: 3.1.4

**D**
- Dead zones: 2.1.2
- Drift mechanics: 2.1.4, 4.3

**F**
- Flight test view: 3.1

**I**
- Input latency: 2.3, 3.1.3
- Inverted controls: 2.1.2

**J**
- Joystick: 2.1

**R**
- Right stick Y-axis: 2.1.2 (MUST be axis 5)

**T**
- Test procedures: 3.1.4
- Triggers: 2.1.3

---

*This specification is a living document. Each section will be expanded with detailed requirements as development progresses.*
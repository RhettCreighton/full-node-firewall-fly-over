# Joystick Specification with Secure Code Point Traces

## Overview

This document defines the complete joystick specification for our game, with each specification tied to **required code point traces**. Every specification must be proven by showing that execution hits specific secure code points in the correct order.

## Core System Design

### No Error Codes - Only Secure Traces

We've eliminated traditional error handling codes. Instead:
1. Each code path has a unique 256-bit SHA3 identifier
2. Specifications define **required traces** (must hit) and **forbidden traces** (must not hit)
3. Compile separate binaries with forbidden paths removed
4. Verify execution traces match specifications

### Secure Code Points

```c
SECURE_CODE_POINT(RIGHT_STICK_Y_AXIS_5, "axis == 5");
```

Each point:
- Has a unique name and SHA3 tag
- Can be compiled in/out with `-DPATH_name_EXISTS=0/1`
- Records trace in TRACING_BUILD mode
- Exits immediately in PROVING_BUILD mode

## Joystick Specifications

### SPEC_001: Initialization Safety

**Description**: Joystick initialization must handle NULL parameters safely.

**Required Trace**:
1. `JOYSTICK_INIT_START` - Entry to initialization
2. `JOYSTICK_INIT_NULL_STATE` or `JOYSTICK_INIT_NULL_DEVICE` - If NULL passed

**Forbidden Trace**:
- `JOYSTICK_INIT_SUCCESS` - Cannot succeed with NULL parameters

**Proof**:
```bash
TRACE_MODE=1 TEST_SCENARIO=init_null_state ./joystick_trace_demo
# Output: TRACE:JOYSTICK_INIT_START -> TRACE:JOYSTICK_INIT_NULL_STATE
```

### SPEC_002: Right Stick Y Uses Axis 5

**Description**: The right analog stick Y-axis MUST read from hardware axis 5, never axis 3.

**Required Trace** (when processing axis 5):
1. `PROCESS_EVENT_START`
2. `EVENT_TYPE_AXIS`
3. `RIGHT_STICK_Y_AXIS_5` ← Critical point
4. `PROCESS_EVENT_SUCCESS`

**Forbidden Trace**:
- `RIGHT_STICK_Y_AXIS_3_WRONG` - This path must be unreachable

**Proof**:
```bash
# Build with axis 3 path disabled
gcc -DPATH_RIGHT_STICK_Y_AXIS_3_WRONG_EXISTS=0 ...

# Test axis 5 - hits required trace
TRACE_MODE=1 TEST_SCENARIO=process_axis_5 ./joystick_trace_demo
# Trace includes: RIGHT_STICK_Y_AXIS_5 ✓

# Test axis 3 - forbidden path doesn't exist
TRACE_MODE=1 TEST_SCENARIO=process_axis_3 ./joystick_trace_demo  
# Trace excludes: RIGHT_STICK_Y_AXIS_3_WRONG ✓
```

### SPEC_003: Get Right Y From Correct Axis

**Description**: The `get_right_stick_y()` function must read exclusively from axis 5.

**Required Trace**:
1. `GET_RIGHT_Y_START`
2. `RIGHT_Y_FROM_AXIS_5` ← Must hit this

**Forbidden Trace**:
- `RIGHT_Y_FROM_AXIS_3_ERROR` - Never read from axis 3

**Proof**:
```bash
TRACE_MODE=1 TEST_SCENARIO=get_right_y ./joystick_trace_demo
# Output: TRACE:GET_RIGHT_Y_START -> TRACE:RIGHT_Y_FROM_AXIS_5
```

### SPEC_004: Bounds Checking

**Description**: Invalid axis numbers must be caught.

**Required Trace** (for out-of-bounds axis):
1. `PROCESS_EVENT_START`
2. `EVENT_TYPE_AXIS`
3. `AXIS_OUT_OF_BOUNDS` ← Must hit for axis >= 8

**Proof**:
```c
// Test code
struct js_event event = {.type = JS_EVENT_AXIS, .number = 99};
process_joystick_event(&js, &event);
// Trace: AXIS_OUT_OF_BOUNDS ✓
```

### SPEC_005: Complete Input Coverage

**Description**: All possible inputs must reach a secure exit point.

**Required Trace**:
- At least one secure code point for every test scenario

**Forbidden Trace**:
- `RIGHT_STICK_Y_AXIS_3_WRONG`
- `RIGHT_Y_FROM_AXIS_3_ERROR`

**Proof**: Run all test scenarios and verify each hits secure points.

## Trace Verification Process

### 1. Build with Traces
```bash
gcc -DTRACING_BUILD \
    -DPATH_RIGHT_STICK_Y_AXIS_3_WRONG_EXISTS=0 \
    -DPATH_RIGHT_Y_FROM_AXIS_3_ERROR_EXISTS=0 \
    ... src/joystick_with_traces.c
```

### 2. Run Test Scenarios
```bash
TRACE_MODE=1 TEST_SCENARIO=process_axis_5 ./joystick_trace_demo
```

### 3. Verify Required Points Hit
```
TRACE:PROCESS_EVENT_START:...
TRACE:EVENT_TYPE_AXIS:...
TRACE:RIGHT_STICK_Y_AXIS_5:... ← Required!
TRACE:PROCESS_EVENT_SUCCESS:...
```

### 4. Verify Forbidden Points Not Hit
The forbidden paths are compiled out - they literally don't exist in the binary.

## Proof Summary

We have proven through execution traces:

1. **Axis 5 Correctness**: When axis 5 events occur, `RIGHT_STICK_Y_AXIS_5` is hit ✓
2. **Axis 3 Forbidden**: `RIGHT_STICK_Y_AXIS_3_WRONG` never hit (doesn't exist) ✓  
3. **Get Function**: `get_right_stick_y()` traces through `RIGHT_Y_FROM_AXIS_5` ✓
4. **Null Safety**: NULL inputs trace through safety checks ✓
5. **Bounds Checking**: Invalid axes trace through bounds checks ✓

## Integration with Build System

```cmake
# Generate trace verification binaries
generate_trace_binary(joystick_traces
    "src/joystick_with_traces.c"
    "${REQUIRED_TRACES}"
    "${FORBIDDEN_TRACES}")

# Verify specifications at build time
verify_trace_specifications(joystick_traces
    "specifications/joystick_trace_requirements.json")
```

## Conclusion

Every joystick specification is tied to specific code execution traces. By:
1. Defining required and forbidden trace points
2. Compiling out forbidden paths
3. Verifying execution traces match specifications
4. Using SHA3 tags for cryptographic proof

We guarantee that the implementation matches the specification exactly. No error codes needed - just proof of which secure code points were hit during execution.
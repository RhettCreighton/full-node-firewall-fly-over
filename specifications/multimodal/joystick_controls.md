# Multimodal Specification: Joystick Controls

## Natural Language
"The right analog stick's Y-axis must read from hardware axis 5, not axis 3, on the ASTRO C40 controller."

## Formal Logic
```
∀ event ∈ JoystickEvents:
  (event.type = AXIS_EVENT ∧ event.axis = 5) → 
    right_stick_y := event.value
```

## Pseudocode
```
WHEN joystick_event_received(event):
  IF event.is_axis_event AND event.axis_number == 5 THEN
    right_stick_y_position = event.axis_value
  END IF
END WHEN
```

## Valid Example
```
Input: js_event{type: JS_EVENT_AXIS, number: 5, value: 16384}
Result: axes[5] = 16384  // Right stick Y updated correctly
Status: ✓ VALID
```

## Invalid Example  
```
Input: js_event{type: JS_EVENT_AXIS, number: 3, value: 16384}
Result: axes[3] = 16384  // Wrong! This updates L2 trigger, not right stick Y
Status: ✗ INVALID - Would cause right stick to not respond
```

## GDB Proof Script
```gdb
# Verify axis 5 is used for right stick Y
break joystick_event_handler if event->number == 3
commands
  print "ERROR: Using axis 3 for right stick!"
  quit 1
end

break joystick_event_handler if event->number == 5  
commands
  print "CORRECT: Using axis 5 for right stick Y"
  continue
end
```

## Unit Test
```c
void test_right_stick_y_axis() {
    js_event event = {
        .type = JS_EVENT_AXIS,
        .number = 5,  // MUST be 5
        .value = 12345
    };
    
    process_joystick_event(&event);
    assert(get_right_stick_y() == 12345);
    assert(get_axis_value(5) == 12345);
    assert(get_axis_value(3) != 12345);  // Axis 3 unchanged
}
```

## Property Test
```
property right_stick_y_always_axis_5:
  forall value in [-32768, 32767]:
    send_axis_event(axis=5, value=value)
    assert get_right_stick_y() == value
    
property axis_3_never_affects_right_stick_y:
  forall value in [-32768, 32767]:
    old_y = get_right_stick_y()
    send_axis_event(axis=3, value=value)
    assert get_right_stick_y() == old_y  // Unchanged
```

## Convergence Check
All eight expressions above describe the SAME requirement:
- Natural language: "axis 5"
- Formal logic: "event.axis = 5"
- Pseudocode: "event.axis_number == 5"
- Valid example: "number: 5"
- Invalid example: "number: 3" (wrong)
- GDB proof: "event->number == 5"
- Unit test: ".number = 5"
- Property test: "axis=5"

ALL converge to: **Right stick Y must use axis index 5**
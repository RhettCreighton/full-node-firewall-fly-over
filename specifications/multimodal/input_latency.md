# Multimodal Specification: Input Latency

## Natural Language
"Input processing must complete within one frame (16.67ms) to maintain smooth 60 FPS gameplay."

## Formal Logic
```
∀ input_event e, timestamp t:
  process_complete(e) - t < 16.67ms
```

## Pseudocode
```
FUNCTION process_input_with_deadline():
  start_time = GET_TIME()
  
  WHILE input_available() AND (GET_TIME() - start_time < 15ms):
    event = poll_input()
    handle_event(event)
  END WHILE
  
  IF GET_TIME() - start_time > 16ms THEN
    LOG_WARNING("Input processing exceeded frame time")
  END IF
END FUNCTION
```

## Valid Example
```c
void input_system_update(float max_ms) {
    double start = get_time_ms();
    
    while (has_input() && (get_time_ms() - start < max_ms)) {
        InputEvent evt = poll_input();  // ✓ Non-blocking
        process_event(&evt);             // ✓ Time bounded
    }
    
    if (get_time_ms() - start > 16.0) {  // ✓ Monitor deadline
        stats.missed_frames++;
    }
}
```

## Invalid Example
```c
void bad_input_loop() {
    while (1) {  // ✗ Infinite loop!
        InputEvent evt = blocking_read();  // ✗ Can block forever!
        process_complex_event(&evt);       // ✗ No time limit!
    }
}
```

## GDB Proof Script
```gdb
# Monitor input processing time
break input_system_update
set $start_time = 0
commands
  set $start_time = $time_now
  continue
end

break input_system_update if $time_now - $start_time > 0.016
commands
  print "Input processing exceeded deadline!"
  continue
end
```

## Unit Test
```c
void test_input_deadline() {
    // Flood with events
    for (int i = 0; i < 10000; i++) {
        push_test_event();
    }
    
    double start = get_time();
    input_system_update(15.0);  // 15ms budget
    double elapsed = get_time() - start;
    
    assert(elapsed < 0.016);  // Must finish in time
}
```

## Property Test
```
property input_always_responsive:
  forall event_count in [0..10000]:
    queue = generate_events(event_count)
    start = current_time()
    process_all_inputs(queue, deadline=16ms)
    elapsed = current_time() - start
    assert elapsed < 17ms  // Small tolerance
```

## Convergence Check
All expressions require:
- Non-blocking input polling
- Time budget enforcement
- Deadline monitoring
- Graceful degradation on overload
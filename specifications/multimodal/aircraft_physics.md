# Multimodal Specification: Aircraft Physics

## Natural Language
"Aircraft physics values must remain within reasonable bounds to prevent numerical instability."

## Formal Logic
```
∀ aircraft a, time t:
  |a.velocity| ≤ MAX_SPEED ∧
  |a.position| ≤ WORLD_SIZE ∧
  ¬isnan(a.position) ∧ ¬isinf(a.position)
```

## Pseudocode
```
FUNCTION update_physics(aircraft, delta_time):
  // Apply forces
  aircraft.velocity += aircraft.acceleration * delta_time
  
  // Clamp velocity
  aircraft.velocity = CLAMP(aircraft.velocity, -MAX_SPEED, MAX_SPEED)
  
  // Update position
  aircraft.position += aircraft.velocity * delta_time
  
  // Ensure in world
  aircraft.position = CLAMP(aircraft.position, -WORLD_SIZE, WORLD_SIZE)
  
  // Verify stability
  ASSERT NOT isnan(aircraft.position)
  ASSERT NOT isinf(aircraft.position)
END FUNCTION
```

## Valid Example
```c
void physics_step(Aircraft *craft, float dt) {
    craft->velocity.x += craft->thrust * dt;
    craft->velocity.x = fminf(fmaxf(craft->velocity.x, -500), 500);  // ✓ Clamped
    
    craft->position.x += craft->velocity.x * dt;
    if (!isfinite(craft->position.x)) {  // ✓ Check for NaN/Inf
        craft->position.x = 0;  // Safe reset
    }
}
```

## Invalid Example
```c
void bad_physics(Aircraft *craft, float dt) {
    craft->velocity.x += craft->thrust * dt;  // ✗ No limits!
    craft->position.x += craft->velocity.x * dt;  // ✗ Can overflow!
    // ✗ No NaN checking!
}
```

## GDB Proof Script
```gdb
# Catch floating point exceptions
handle SIGFPE stop
break update_physics
commands
  silent
  if $craft->velocity.x > 1000 || $craft->velocity.x < -1000
    print "Velocity exceeds safe bounds!"
    quit 1
  end
  continue
end
```

## Unit Test
```c
void test_physics_bounds() {
    Aircraft craft = {0};
    craft.thrust = 10000;  // Extreme thrust
    
    for (int i = 0; i < 1000; i++) {
        physics_step(&craft, 0.016f);
        assert(fabs(craft.velocity.x) <= MAX_SPEED);
        assert(isfinite(craft.position.x));
    }
}
```

## Property Test
```
property physics_always_stable:
  forall thrust in [-10000..10000]:
    forall dt in [0.001..1.0]:
      aircraft = create_aircraft()
      aircraft.thrust = thrust
      for 1000 iterations:
        update_physics(aircraft, dt)
      assert isfinite(aircraft.position)
      assert magnitude(aircraft.velocity) <= MAX_SPEED
```

## Convergence Check
All expressions require:
- Velocity clamping to MAX_SPEED
- Position bounds checking
- NaN/Inf detection and handling
- Stable under extreme inputs
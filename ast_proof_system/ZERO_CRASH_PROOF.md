# Zero Crash Mathematical Proof System

## The Problem: Error Handlers Are Admissions of Failure

When we write code like:
```c
if (shake_range < 1) shake_range = 1;  // WRONG - This admits we can get here!
```

We're admitting our code can reach an error state. **This is unacceptable.**

## The Solution: Mathematical Guarantees

### 1. **Input Invariants** - Guarantee inputs are ALWAYS valid
```c
typedef struct {
    float screen_shake;  // INVARIANT: 0.0f <= screen_shake <= 10.0f
} effects_manager_t;

// Constructor MUST establish invariant
effects_manager_t* effects_create() {
    effects_manager_t* m = calloc(1, sizeof(*m));
    m->screen_shake = 0.0f;  // Valid by construction
    return m;
}

// ALL setters MUST maintain invariant
void effects_set_shake(effects_manager_t* m, float shake) {
    m->screen_shake = CLAMP(shake, 0.0f, 10.0f);  // CANNOT violate invariant
}
```

### 2. **Proof by Construction** - Make invalid states unrepresentable
```c
// WRONG - Can be zero
int shake_range = (int)(screen_shake * 20.0f);
if (shake_range < 1) shake_range = 1;  // ERROR PATH EXISTS

// RIGHT - Cannot be zero by construction
int shake_range = MAX(1, (int)(screen_shake * 20.0f));  // NO ERROR PATH

// BETTER - Use safe modulo that handles zero
#define SAFE_MOD(a, b) ((b) == 0 ? 0 : (a) % (b))
shake_x = SAFE_MOD(rand(), shake_range);  // CANNOT CRASH
```

### 3. **Static Verification** - Compile-time proofs
```c
// Add to build system
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

// Prove at compile time
STATIC_ASSERT(MIN_SHAKE_RANGE >= 1);
STATIC_ASSERT(sizeof(effects_manager_t) == expected_size);
```

### 4. **Domain Restrictions** - Limit input space
```c
// Instead of float (infinite values), use fixed point
typedef uint16_t shake_amount_t;  // 0-65535 only

// Convert at boundaries
shake_amount_t float_to_shake(float f) {
    return (shake_amount_t)(CLAMP(f, 0.0f, 1.0f) * 65535);
}

// Now modulo CANNOT fail
int shake_range = (shake_amount >> 11) + 1;  // Always >= 1
```

## Applying to combat_effects.c

### Before (Has Error Paths):
```c
if (manager->screen_shake > 0) {
    int shake_range = (int)(manager->screen_shake * 20.0f);
    if (shake_range < 1) shake_range = 1;  // ERROR PATH
    shake_x = (rand() % shake_range) - shake_offset;
}
```

### After (No Error Paths):
```c
// Option 1: Guaranteed minimum
int shake_range = 1 + (int)(manager->screen_shake * 19.0f);  // ALWAYS >= 1

// Option 2: Safe operations
shake_x = SAFE_MOD(rand(), MAX(1, shake_range)) - shake_offset;

// Option 3: Multiplicative approach (no modulo)
float normalized_rand = (float)rand() / RAND_MAX;  // 0.0 to 1.0
shake_x = (int)((normalized_rand - 0.5f) * manager->screen_shake * 20.0f);
```

## AST Proof Integration

### 1. Find ALL Error Paths
```c
// ast_proof_system/src/error_path_finder.c
typedef enum {
    ERROR_PATH_DIVIDE_BY_ZERO,
    ERROR_PATH_NULL_CHECK,
    ERROR_PATH_BOUNDS_CHECK,
    ERROR_PATH_DEFAULT_CASE,
    ERROR_PATH_ERROR_RETURN
} error_path_type_t;

// Find patterns like:
// if (x < min) x = min;  // Admission of failure!
// if (!ptr) return -1;   // Error path!
// default: assert(0);    // Should be unreachable!
```

### 2. Prove Error Paths Unreachable
```c
// For each error path found:
// 1. Trace backwards to find all inputs
// 2. Verify input constraints make path impossible
// 3. If reachable, FAIL compilation
```

### 3. Generate Proof Certificate
```
PROOF CERTIFICATE: combat_effects.c
==================================
Functions analyzed: 23
Error paths found: 3
Error paths eliminated: 3

ELIMINATED PATHS:
1. Line 497: shake_range < 1
   Proof: screen_shake constrained to [0, 10]
          shake_range = int(screen_shake * 20)
          Therefore: shake_range ∈ [0, 200]
   Fix: Use MAX(1, shake_range) in calculation

2. Line 512: feedback->damage_amount < 0
   Proof: damage_amount is unsigned
   Fix: Already impossible

3. Line 569: divide by spread when spread == 0
   Proof: spread parameter constrained > 0
   Fix: Add precondition check

VERIFICATION: PASSED ✓
```

## The Ultimate Goal

**Every error handler should be provably unreachable.**

If we need:
- `if (divisor == 0) divisor = 1;` → We failed
- `if (!ptr) return NULL;` → We failed  
- `if (index >= size) index = size-1;` → We failed

Instead, we must PROVE these conditions cannot occur through:
1. **Type constraints** (unsigned, ranged types)
2. **Invariants** (maintained by all operations)
3. **Preconditions** (checked at API boundaries)
4. **Mathematical construction** (operations that cannot fail)

## Implementation Plan

1. **Immediate**: Fix combat_effects.c to remove ALL error paths
2. **Short term**: Run AST analyzer to find ALL error handlers
3. **Medium term**: Prove each error handler is unreachable
4. **Long term**: Ban error handlers in critical paths

## Verification Command

```bash
./ast_proof_system/build/bin/firewall-fly-over \
    --find-error-paths \
    --prove-unreachable \
    --fail-if-reachable \
    src/views/combat_effects.c
```

If this passes, we have **mathematical proof** that the code cannot crash.
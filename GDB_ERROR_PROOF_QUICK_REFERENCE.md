# GDB Error Proof - Quick Reference Card

## 🚀 Setup (One Time)
```cmake
add_subdirectory(external/gdb-error-proof)
target_link_libraries(your_app PRIVATE gdb_error_proof)
```

```c
#include <gdb-error-proof/safety.h>
```

## 🛡️ Most Used Functions

### Array Safety
```c
// Turn ANY index into valid range [0, size-1]
size_t safe_idx = safe_array_index(user_input, array_size);
buffer[safe_idx] = value;  // ALWAYS safe
```

### Division Safety  
```c
// Never crashes, returns 0 if denominator is 0/NaN/Inf
float result = SAFE_DIVIDE(a, b);
```

### Input Clamping
```c
// Force value into range [min, max]
float safe = safe_clamp_float(user_input, 0.0f, 100.0f);
```

### Safe Math
```c
// Overflow-safe operations (saturate at limits)
int sum = SAFE_ADD(a, b);
int diff = SAFE_SUB(a, b);  
int prod = SAFE_MUL(a, b);
```

## 📋 Copy-Paste Templates

### Template 1: Safe User Input
```c
void process_click(int mouse_x, int mouse_y) {
    size_t x = safe_array_index(mouse_x, screen_width);
    size_t y = safe_array_index(mouse_y, screen_height);
    pixels[y * screen_width + x] = RED;
}
```

### Template 2: Safe Config Values
```c
void load_settings(float user_values[], int count) {
    for (int i = 0; i < MAX_SETTINGS; i++) {
        size_t idx = safe_array_index(i, count);
        settings[i] = safe_clamp_float(user_values[idx], 0.0f, 1.0f);
    }
}
```

### Template 3: Safe Game Loop
```c
void game_update(float delta_time) {
    float fps = SAFE_DIVIDE(1.0f, delta_time);
    float speed = safe_clamp_float(player_speed, 0.0f, MAX_SPEED);
    player_pos = SAFE_ADD(player_pos, SAFE_MUL(speed, delta_time));
}
```

## ⚡ One-Liners

```c
// Safe percentage calculation
float percent = SAFE_DIVIDE(current * 100.0f, total);

// Safe average
float avg = SAFE_DIVIDE(sum, count);

// Safe array wrap-around
next_slot = safe_array_index(current_slot + 1, MAX_SLOTS);

// Safe sensitivity scaling
final_sens = safe_clamp_float(base_sens * multiplier, 0.1f, 10.0f);
```

## 🎯 Remember

**Before**: `array[index]` → 💥 Crash if index < 0 or >= size  
**After**: `array[safe_array_index(index, size)]` → ✅ Always safe

**Before**: `a / b` → 💥 Crash if b is 0  
**After**: `SAFE_DIVIDE(a, b)` → ✅ Returns 0 if b is 0

**Before**: `big_int + big_int` → 💥 Undefined overflow  
**After**: `SAFE_ADD(big_int, big_int)` → ✅ Saturates at INT_MAX
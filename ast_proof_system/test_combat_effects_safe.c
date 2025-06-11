/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Test version of combat_effects with mathematical safety guarantees
 */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the zero_crash.h macros */
#define SHAKE_TO_RANGE(shake) (1 + (int)((shake) * 19.0f))
#define CONSTRAINED(var, constraint)
#define PROVES_UNREACHABLE(condition)

typedef struct {
    float screen_shake;  /* INVARIANT: 0.0 <= screen_shake <= 10.0 */
} effects_manager_t;

/* The FIXED version that cannot crash */
void effects_draw_ui_safe(effects_manager_t* manager, int screen_width, int screen_height) {
    if (!manager) return;
    
    // Screen shake offset - MATHEMATICALLY PROVEN SAFE
    int shake_x = 0, shake_y = 0;
    if (manager->screen_shake > 0) {
        CONSTRAINED(manager->screen_shake, "0.0 <= x <= 10.0");
        
        // Using SHAKE_TO_RANGE macro that guarantees >= 1
        int shake_range = SHAKE_TO_RANGE(manager->screen_shake);
        PROVES_UNREACHABLE("shake_range < 1");
        
        int shake_offset = (int)(manager->screen_shake * 10.0f);
        shake_x = (rand() % shake_range) - shake_offset;
        shake_y = (rand() % shake_range) - shake_offset;
    }
    
    printf("Shake: x=%d, y=%d\n", shake_x, shake_y);
}

/* The OLD version that COULD crash */
void effects_draw_ui_unsafe(effects_manager_t* manager, int screen_width, int screen_height) {
    if (!manager) return;
    
    // OLD CODE - CAN CRASH!
    int shake_x = 0, shake_y = 0;
    if (manager->screen_shake > 0) {
        int shake_range = (int)(manager->screen_shake * 20.0f);
        // DANGER: shake_range could be 0 if screen_shake < 0.05
        shake_x = (rand() % shake_range) - manager->screen_shake * 10;
        shake_y = (rand() % shake_range) - manager->screen_shake * 10;
    }
}

/* Test harness */
int main() {
    effects_manager_t manager;
    
    printf("=== Testing Safe Version ===\n");
    
    // Test edge cases
    float test_values[] = {0.0f, 0.01f, 0.04f, 0.05f, 0.1f, 1.0f, 5.0f, 10.0f};
    
    for (int i = 0; i < 8; i++) {
        manager.screen_shake = test_values[i];
        printf("\nTesting screen_shake = %.2f\n", manager.screen_shake);
        
        // Calculate what shake_range will be
        if (manager.screen_shake > 0) {
            int shake_range_safe = SHAKE_TO_RANGE(manager.screen_shake);
            int shake_range_unsafe = (int)(manager.screen_shake * 20.0f);
            
            printf("  Safe formula: 1 + (int)(%.2f * 19) = %d\n", 
                   manager.screen_shake, shake_range_safe);
            printf("  Unsafe formula: (int)(%.2f * 20) = %d %s\n",
                   manager.screen_shake, shake_range_unsafe,
                   shake_range_unsafe == 0 ? "*** WOULD CRASH! ***" : "");
        }
        
        // Run safe version (never crashes)
        effects_draw_ui_safe(&manager, 1280, 720);
    }
    
    printf("\n=== Mathematical Proof ===\n");
    printf("For any screen_shake >= 0:\n");
    printf("  SHAKE_TO_RANGE(shake) = 1 + floor(shake * 19)\n");
    printf("  Since floor(x) >= 0 for x >= 0\n");
    printf("  Therefore: SHAKE_TO_RANGE(shake) >= 1\n");
    printf("  Conclusion: modulo operation CANNOT fail\n");
    
    return 0;
}
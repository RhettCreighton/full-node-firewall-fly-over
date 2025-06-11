/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/specification_enforcement.h"
#include "sky_combat/utils/safety_macros.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Example: How specifications make violations IMPOSSIBLE
 */

// ============================================================================
// SPECIFICATION: Functions must check preconditions
// ============================================================================

// OLD WAY (can violate specification):
void process_data_unsafe(int* data, size_t size) {
    // BUG: No null check, no size check
    for (size_t i = 0; i < size; i++) {
        data[i] *= 2;  // CRASH if data is NULL!
    }
}

// NEW WAY (specification enforced):
void process_data_safe(int* data, size_t size) {
    // Preconditions ENFORCED - cannot be skipped
    SPEC_REQUIRES(data != NULL);
    SPEC_REQUIRES(size > 0);
    
    SPEC_CRITICAL_SECTION_BEGIN("data_processing");
    
    for (size_t i = 0; i < size; i++) {
        // Even in loop, bounds are monitored
        SPEC_MONITOR_RANGE(i, 0, size - 1);
        data[i] *= 2;
    }
    
    SPEC_CRITICAL_SECTION_END("data_processing");
    
    // Postcondition verified
    SPEC_ENSURES(data[0] % 2 == 0);
}

// ============================================================================
// SPECIFICATION: No division by zero
// ============================================================================

// OLD WAY (can crash):
int calculate_average_unsafe(int sum, int count) {
    return sum / count;  // CRASH if count is 0!
}

// NEW WAY (specification enforced):
int calculate_average_safe(int sum, int count) {
    // Cannot divide by zero - macro enforces specification
    return SPEC_SAFE_DIV(sum, count);
}

// ============================================================================
// SPECIFICATION: Array access must be bounds-checked
// ============================================================================

// OLD WAY (buffer overflow):
void update_scores_unsafe(int scores[], int index, int value) {
    scores[index] = value;  // CRASH if index >= array size!
}

// NEW WAY (specification enforced):
void update_scores_safe(int scores[], int index, int value, size_t size) {
    // Cannot overflow - macro enforces bounds
    SPEC_SAFE_ARRAY(scores, index, size) = value;
    
    // Runtime verification
    SPEC_VERIFY("score_in_range", value >= 0 && value <= 100);
}

// ============================================================================
// SPECIFICATION: Pointers must be valid before use
// ============================================================================

typedef struct {
    int x, y;
    int health;
} Entity;

// OLD WAY (null pointer dereference):
int get_health_unsafe(Entity* entity) {
    return entity->health;  // CRASH if entity is NULL!
}

// NEW WAY (specification enforced):
int get_health_safe(Entity* entity) {
    // Cannot dereference NULL - macro provides safe fallback
    return SPEC_SAFE_DEREF(entity, health);
}

// ============================================================================
// SPECIFICATION: Critical invariants must hold
// ============================================================================

typedef struct {
    int current;
    int max;
} HealthSystem;

void damage_entity(HealthSystem* health, int damage) {
    SPEC_REQUIRES(health != NULL);
    SPEC_REQUIRES(damage >= 0);
    
    // Invariant: health is always between 0 and max
    SPEC_INVARIANT(health->current >= 0);
    SPEC_INVARIANT(health->current <= health->max);
    
    health->current -= damage;
    
    // Enforce invariant even after damage
    if (health->current < 0) {
        health->current = 0;
    }
    
    // Verify invariant still holds
    SPEC_INVARIANT(health->current >= 0);
    SPEC_INVARIANT(health->current <= health->max);
}

// ============================================================================
// COMPILE-TIME SPECIFICATION ENFORCEMENT
// ============================================================================

// This won't compile - specifications prevent it:
#if 0
void will_not_compile() {
    char buffer[10];
    strcpy(buffer, "too long!");  // COMPILE ERROR: Use SPEC_SAFE_STRCPY
    
    int* p = malloc(100);
    *p = 42;  // COMPILE ERROR: Check malloc result
    
    int x = 10 / 0;  // COMPILE ERROR: Use SPEC_SAFE_DIV
}
#endif

// ============================================================================
// DEMONSTRATION
// ============================================================================

int main() {
    printf("=== Specification Enforcement Demo ===\n\n");
    
    // Initialize enforcement system
    spec_enforcement_init();
    
    // Test 1: Safe operations
    printf("Test 1: Safe operations\n");
    int data[] = {1, 2, 3, 4, 5};
    process_data_safe(data, 5);
    printf("✓ Data processed safely\n");
    
    // Test 2: Division safety
    printf("\nTest 2: Division by zero protection\n");
    int avg = calculate_average_safe(100, 0);  // Would crash without protection
    printf("✓ Average with 0 count = %d (safe fallback)\n", avg);
    
    // Test 3: Array bounds
    printf("\nTest 3: Array bounds protection\n");
    int scores[3] = {0};
    update_scores_safe(scores, 10, 100, 3);  // Would overflow without protection
    printf("✓ Out-of-bounds access prevented\n");
    
    // Test 4: Null pointer safety
    printf("\nTest 4: Null pointer protection\n");
    Entity* null_entity = NULL;
    int health = get_health_safe(null_entity);  // Would crash without protection
    printf("✓ Null pointer handled safely, health = %d\n", health);
    
    // Test 5: Invariant enforcement
    printf("\nTest 5: Invariant enforcement\n");
    HealthSystem player_health = {100, 100};
    damage_entity(&player_health, 150);  // Would violate invariant without protection
    printf("✓ Health invariant maintained: %d/%d\n", 
           player_health.current, player_health.max);
    
    printf("\n=== All specifications enforced successfully! ===\n");
    
    return 0;
}
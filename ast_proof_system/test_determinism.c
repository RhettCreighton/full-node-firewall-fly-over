/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Test file demonstrating deterministic vs non-deterministic operations
 * Shows how constraint verification makes proofs stronger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* DETERMINISTIC OPERATIONS - Can be proven once */

// Pure mathematical function - always deterministic
float calculate_physics(float mass, float velocity) {
    // This is deterministic - same inputs always give same output
    float kinetic_energy = 0.5f * mass * velocity * velocity;
    
    // Safe division - deterministic with compile-time check
    if (mass > 0.0f) {
        float specific_energy = kinetic_energy / mass;
        return specific_energy;
    }
    return 0.0f;
}

// Array operation with compile-time bounds
int process_static_array(void) {
    static const int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int sum = 0;
    
    // Deterministic loop with known bounds
    for (int i = 0; i < 10; i++) {
        sum += data[i];  // Provably safe - compile time bounds
    }
    
    return sum;  // Always returns 55
}

/* NON-DETERMINISTIC OPERATIONS - Need constraints */

// User input - non-deterministic
float process_user_input(void) {
    float value;
    printf("Enter a value: ");
    scanf("%f", &value);  // Non-deterministic input!
    
    // UNCONSTRAINED - This is dangerous!
    float result = 100.0f / value;  // Could divide by zero!
    
    return result;
}

// User input with proper constraints
float process_user_input_safe(void) {
    float value;
    printf("Enter a value: ");
    scanf("%f", &value);  // Non-deterministic input
    
    // CONSTRAINED - Input is bounded
    if (value < 0.1f) value = 0.1f;  // Clamp minimum
    if (value > 100.0f) value = 100.0f;  // Clamp maximum
    
    // Now safe - value is guaranteed to be in [0.1, 100.0]
    float result = 100.0f / value;  // Cannot divide by zero
    
    return result;
}

// Random values - non-deterministic
void process_random_array(int* array, int size) {
    // Fill with random values
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 1000;  // Non-deterministic!
    }
    
    // UNCONSTRAINED array access
    int random_index = rand() % 1000;  // Could be >= size!
    int value = array[random_index];  // DANGEROUS - out of bounds!
}

// Random with proper constraints
void process_random_array_safe(int* array, int size) {
    // Validate size first
    if (size <= 0 || array == NULL) return;
    
    // Fill with random values
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 1000;
    }
    
    // CONSTRAINED array access
    int random_index = rand() % size;  // Guaranteed to be in bounds
    int value = array[random_index];  // Safe access
}

// Time-based operation - non-deterministic
float calculate_time_dependent(void) {
    time_t current_time = time(NULL);  // Non-deterministic!
    
    // Using time as divisor - dangerous!
    int seconds = current_time % 60;
    float result = 3600.0f / seconds;  // Could divide by zero when seconds=0!
    
    return result;
}

// Time-based with constraints
float calculate_time_dependent_safe(void) {
    time_t current_time = time(NULL);  // Non-deterministic
    
    // CONSTRAINED - ensure non-zero
    int seconds = (current_time % 60) + 1;  // Now in range [1, 60]
    float result = 3600.0f / seconds;  // Safe division
    
    return result;
}

/* MIXED DETERMINISM - Partially deterministic */

// Function with both deterministic and non-deterministic parts
float mixed_calculation(float base_value) {
    // Deterministic part
    float squared = base_value * base_value;
    
    // Non-deterministic part
    float random_factor = (float)rand() / RAND_MAX;  // [0, 1]
    
    // Constrain the random factor
    random_factor = 0.5f + random_factor * 0.5f;  // Now [0.5, 1.0]
    
    // Safe division with constrained non-deterministic value
    float result = squared / random_factor;  // Cannot be zero
    
    return result;
}

/* GAME-SPECIFIC EXAMPLES */

// Player input - non-deterministic
typedef struct {
    float x, y;
} Vector2;

Vector2 get_player_movement(void) {
    // Simulate gamepad input (non-deterministic)
    Vector2 input;
    input.x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;  // [-1, 1]
    input.y = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;  // [-1, 1]
    
    // Normalize - UNCONSTRAINED!
    float magnitude = sqrtf(input.x * input.x + input.y * input.y);
    input.x /= magnitude;  // Could divide by zero!
    input.y /= magnitude;
    
    return input;
}

Vector2 get_player_movement_safe(void) {
    // Simulate gamepad input (non-deterministic)
    Vector2 input;
    input.x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    input.y = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    
    // Safe normalization with constraint
    float magnitude = sqrtf(input.x * input.x + input.y * input.y);
    if (magnitude < 0.001f) {
        // Dead zone - return zero vector
        input.x = 0.0f;
        input.y = 0.0f;
    } else {
        // Safe to normalize
        input.x /= magnitude;
        input.y /= magnitude;
    }
    
    return input;
}

int main() {
    printf("Determinism and Constraint Test\n");
    
    // Deterministic operations - can be cached/eliminated
    float physics = calculate_physics(10.0f, 5.0f);
    int sum = process_static_array();
    
    // Non-deterministic - need runtime verification
    srand(time(NULL));
    float time_calc = calculate_time_dependent_safe();
    
    printf("Results: physics=%f, sum=%d, time=%f\n", physics, sum, time_calc);
    
    return 0;
}
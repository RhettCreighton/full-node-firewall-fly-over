/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Six Sigma Safety Demo
 * Shows how to make code that CANNOT crash from bad input
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "six_sigma_safety.h"

/* Game simulation with six sigma safety */
typedef struct {
    vector3_t position;
    vector3_t velocity;
    float health;
    int ammo;
    int state;
} player_t;

/* Initialize player with safe defaults */
void player_init(player_t* p) {
    p->position = (vector3_t){0.0f, 0.0f, 0.0f};
    p->velocity = (vector3_t){0.0f, 0.0f, 0.0f};
    p->health = 100.0f;
    p->ammo = 30;
    p->state = 0;
}

/* Process ANY user input safely */
void player_handle_input(player_t* p, const void* raw_input, size_t input_size) {
    /* Sanitize ALL input through six sigma system */
    sanitized_input_t input = sanitize_any_input(raw_input, input_size);
    
    /* Movement - CANNOT produce invalid velocity */
    p->velocity.x = safe_clamp_float(input.values[0], -10.0f, 10.0f);
    p->velocity.y = safe_clamp_float(input.values[1], -10.0f, 10.0f);
    p->velocity.z = safe_clamp_float(input.values[2], -5.0f, 5.0f);
    
    /* Normalize if needed - CANNOT fail */
    p->velocity = safe_normalize_vector(p->velocity);
    
    /* State transition - CANNOT produce invalid state */
    p->state = safe_state_transition(p->state, input.indices[0], 5);
}

/* Update player physics - CANNOT crash */
void player_update(player_t* p, float raw_dt) {
    /* Sanitize delta time */
    float dt = safe_delta_time(raw_dt);
    
    /* Update position - CANNOT overflow */
    p->position.x += p->velocity.x * dt;
    p->position.y += p->velocity.y * dt;
    p->position.z += p->velocity.z * dt;
    
    /* Clamp to world bounds */
    p->position.x = safe_clamp_float(p->position.x, -1000.0f, 1000.0f);
    p->position.y = safe_clamp_float(p->position.y, -1000.0f, 1000.0f);
    p->position.z = safe_clamp_float(p->position.z, 0.0f, 500.0f);
}

/* Calculate damage - CANNOT divide by zero */
float calculate_damage(float base_damage, float armor, float distance) {
    /* Sanitize inputs */
    base_damage = safe_clamp_float(base_damage, 0.0f, 1000.0f);
    armor = safe_clamp_float(armor, 0.1f, 100.0f);  /* Never zero */
    distance = safe_clamp_float(distance, 0.1f, 1000.0f);
    
    /* Safe calculations */
    float armor_reduction = SAFE_DIVIDE(base_damage, armor);
    float distance_falloff = SAFE_DIVIDE(100.0f, distance);
    
    return armor_reduction * distance_falloff;
}

/* Array access example - CANNOT go out of bounds */
void process_inventory(int* items, size_t count, int user_index) {
    /* Safe index */
    size_t index = safe_array_index(user_index, count);
    
    /* This CANNOT crash */
    int item = items[index];
    printf("Selected item at index %zu: %d\n", index, item);
}

/* Demonstrate all safety features */
int main() {
    printf("=== Six Sigma Safety Demo ===\n\n");
    
    /* Test 1: NaN/Inf handling */
    printf("Test 1: NaN/Inf Safety\n");
    float nan_value = 0.0f / 0.0f;  /* NaN */
    float inf_value = 1.0f / 0.0f;  /* Infinity */
    
    float safe_nan = safe_clamp_float(nan_value, 0.0f, 1.0f);
    float safe_inf = safe_clamp_float(inf_value, 0.0f, 1.0f);
    
    printf("  NaN input -> %.2f (safe)\n", safe_nan);
    printf("  Inf input -> %.2f (safe)\n", safe_inf);
    
    /* Test 2: Division safety */
    printf("\nTest 2: Division Safety\n");
    float result1 = SAFE_DIVIDE(100.0f, 0.0f);
    float result2 = SAFE_DIVIDE(100.0f, 5.0f);
    printf("  100/0 = %.2f (safe)\n", result1);
    printf("  100/5 = %.2f (correct)\n", result2);
    
    /* Test 3: Array bounds safety */
    printf("\nTest 3: Array Bounds Safety\n");
    int items[5] = {10, 20, 30, 40, 50};
    process_inventory(items, 5, -10);    /* Negative index */
    process_inventory(items, 5, 100);    /* Out of bounds */
    process_inventory(items, 5, 3);      /* Valid index */
    
    /* Test 4: Vector normalization safety */
    printf("\nTest 4: Vector Normalization Safety\n");
    vector3_t zero_vec = {0.0f, 0.0f, 0.0f};
    vector3_t normal_vec = {3.0f, 4.0f, 0.0f};
    
    vector3_t safe_zero = safe_normalize_vector(zero_vec);
    vector3_t safe_normal = safe_normalize_vector(normal_vec);
    
    printf("  Zero vector -> (%.2f, %.2f, %.2f)\n", 
           safe_zero.x, safe_zero.y, safe_zero.z);
    printf("  Normal vector -> (%.2f, %.2f, %.2f)\n",
           safe_normal.x, safe_normal.y, safe_normal.z);
    
    /* Test 5: Game simulation */
    printf("\nTest 5: Game Simulation\n");
    player_t player;
    player_init(&player);
    
    /* Simulate random hostile input */
    srand(time(NULL));
    for (int i = 0; i < 5; i++) {
        /* Generate hostile input */
        float hostile_input[8];
        for (int j = 0; j < 8; j++) {
            int hostile_type = rand() % 4;
            switch (hostile_type) {
                case 0: hostile_input[j] = INFINITY; break;
                case 1: hostile_input[j] = -INFINITY; break;
                case 2: hostile_input[j] = NAN; break;
                case 3: hostile_input[j] = (float)(rand() - RAND_MAX/2); break;
            }
        }
        
        /* Process hostile input - CANNOT crash */
        player_handle_input(&player, hostile_input, sizeof(hostile_input));
        player_update(&player, hostile_input[7]);  /* Random dt */
        
        printf("  Frame %d: pos=(%.1f,%.1f,%.1f) state=%d\n",
               i, player.position.x, player.position.y, player.position.z, player.state);
    }
    
    /* Test 6: Damage calculation */
    printf("\nTest 6: Damage Calculation Safety\n");
    float damage1 = calculate_damage(100.0f, 0.0f, 0.0f);     /* Zero armor and distance */
    float damage2 = calculate_damage(INFINITY, -10.0f, NAN);  /* Hostile values */
    float damage3 = calculate_damage(50.0f, 10.0f, 20.0f);    /* Normal values */
    
    printf("  Damage with zeros: %.2f\n", damage1);
    printf("  Damage with hostile: %.2f\n", damage2);
    printf("  Damage normal: %.2f\n", damage3);
    
    printf("\n=== All Tests Passed - No Crashes! ===\n");
    
    return 0;
}
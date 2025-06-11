/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Test file demonstrating six sigma safety integration
 * All dangerous operations are protected
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "six_sigma_safety.h"

/* Use vector3_t from six_sigma_safety.h */
typedef vector3_t Vector3;

typedef struct {
    int id;
    char name[32];
    Vector3 position;
    float health;
} GameObject;

/* SAFE: Division protected */
float calculate_damage(float base_damage, float armor) {
    /* Six sigma protection */
    base_damage = safe_clamp_float(base_damage, 0.0f, 1000.0f);
    armor = safe_clamp_float(armor, 0.1f, 100.0f);  /* Never zero */
    
    return SAFE_DIVIDE(base_damage, armor);
}

/* SAFE: Null pointer protected */
void update_object(GameObject* obj) {
    if (!obj) return;  /* Null check */
    
    obj->position.x += 1.0f;
}

/* SAFE: Array bounds protected */
void process_items(int items[], int count) {
    for (int i = 0; i < count; i++) {
        int safe_idx = safe_array_index(i, count);
        items[safe_idx] = i * 10;
    }
}

/* SAFE: Vector normalization protected */
Vector3 normalize_vector(Vector3 v) {
    return safe_normalize_vector(v);
}

/* SAFE: All math protected */
float complex_calculation(float a, float b, float c) {
    float safe_a = safe_clamp_float(a, -100.0f, 100.0f);
    float safe_b = safe_clamp_float(b, 0.1f, 100.0f);
    float safe_c = safe_clamp_float(c, 0.1f, 100.0f);
    
    float result1 = SAFE_DIVIDE(safe_a, safe_b);
    float result2 = SAFE_DIVIDE(result1, safe_c);
    
    return safe_clamp_float(result2, -1000.0f, 1000.0f);
}

/* SAFE: Time delta protected */
void update_physics(GameObject* obj, float raw_dt) {
    if (!obj) return;
    
    float dt = safe_delta_time(raw_dt);
    
    obj->position.x += obj->position.x * dt;
    obj->position.y += obj->position.y * dt;
    
    /* Clamp to world bounds */
    obj->position.x = safe_clamp_float(obj->position.x, -1000.0f, 1000.0f);
    obj->position.y = safe_clamp_float(obj->position.y, -1000.0f, 1000.0f);
}

int main() {
    printf("Safe code example - all operations protected\n");
    
    /* Test hostile inputs */
    float damage = calculate_damage(INFINITY, 0.0f);
    printf("Damage with hostile input: %.2f\n", damage);
    
    Vector3 zero_vec = {0.0f, 0.0f, 0.0f};
    Vector3 normalized = normalize_vector(zero_vec);
    printf("Normalized zero vector: (%.2f, %.2f, %.2f)\n",
           normalized.x, normalized.y, normalized.z);
    
    return 0;
}
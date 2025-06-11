/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Test file with known dangerous operations
 * Used to validate AST analyzer catches all issues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alloca.h>

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    int id;
    char name[32];
    Vector3 position;
    float health;
} GameObject;

/* TEST 1: Division by zero */
float calculate_damage(float base_damage, float armor) {
    // BUG: No check for armor == 0
    return base_damage / armor;
}

/* TEST 2: Null pointer dereference */
void update_object(GameObject* obj) {
    // BUG: No NULL check
    obj->position.x += 1.0f;
}

/* TEST 3: Array bounds violation */
void process_items(int items[], int count) {
    // BUG: Off-by-one error
    for (int i = 0; i <= count; i++) {
        items[i] = i * 2;
    }
}

/* TEST 4: Buffer overflow */
void set_name(GameObject* obj, const char* name) {
    // BUG: No bounds check
    strcpy(obj->name, name);
}

/* TEST 5: Vector normalization */
Vector3 normalize_vector(Vector3 v) {
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    // BUG: No zero check
    return (Vector3){v.x / length, v.y / length, v.z / length};
}

/* TEST 6: Square root of negative */
float calculate_distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    // BUG: Could be negative due to floating point errors
    return sqrtf(dx * dx - dy * dy);  // Should be + not -
}

/* TEST 7: Inverse trig domain error */
float calculate_angle(float dot_product) {
    // BUG: No clamping to [-1, 1]
    return acosf(dot_product);
}

/* TEST 8: Integer overflow */
int calculate_score(int kills, int multiplier) {
    // BUG: Can overflow
    return kills * multiplier * 1000;
}

/* TEST 9: Memory allocation failure */
GameObject* create_object() {
    // BUG: No NULL check
    GameObject* obj = malloc(sizeof(GameObject));
    obj->id = 0;  // Crash if malloc failed
    return obj;
}

/* TEST 10: Double free */
void cleanup_objects(GameObject** objects, int count) {
    for (int i = 0; i < count; i++) {
        free(objects[i]);
        // BUG: Not setting to NULL
    }
    // Later code might free again
}

/* TEST 11: Use after free */
void process_and_free(GameObject* obj) {
    free(obj);
    // BUG: Using after free
    if (obj->health > 0) {
        printf("Object still alive\n");
    }
}

/* TEST 12: Stack overflow (alloca) */
void process_dynamic_array(int size) {
    // BUG: No size limit
    int* array = alloca(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        array[i] = i;
    }
}

/* TEST 13: Float to int overflow */
void set_position(GameObject* obj, float x, float y) {
    // BUG: No range check
    int grid_x = (int)x;  // Could overflow if x > INT_MAX
    int grid_y = (int)y;
}

/* TEST 14: Modulo by zero */
int get_wrapped_index(int index, int wrap) {
    // BUG: No zero check
    return index % wrap;
}

/* TEST 15: Pointer arithmetic overflow */
void process_buffer(char* buffer, int offset) {
    // BUG: No bounds check
    char* ptr = buffer + offset;  // Could overflow
    *ptr = 0;
}

/* TEST 16: Matrix inversion (game specific) */
typedef struct {
    float m[16];
} Matrix4;

Matrix4 invert_matrix(Matrix4 mat) {
    // BUG: No determinant check
    // Simplified - would have actual inversion code
    float det = mat.m[0];  // Fake determinant
    Matrix4 result;
    for (int i = 0; i < 16; i++) {
        result.m[i] = mat.m[i] / det;  // Divide by zero if singular
    }
    return result;
}

/* TEST 17: Sound resource (game specific) */
typedef struct {
    int id;
    void* data;
} Sound;

void play_sound(Sound* sound) {
    // BUG: No NULL check on sound or sound->data
    printf("Playing sound %d\n", sound->id);
}

/* TEST 18: Texture loading (game specific) */
typedef struct {
    int width, height;
    unsigned char* pixels;
} Texture;

void render_texture(Texture* tex, int x, int y) {
    // BUG: No validation
    int pixel_index = y * tex->width + x;  // Could overflow
    unsigned char pixel = tex->pixels[pixel_index];  // Could crash
}

/* TEST 19: Safe examples (should NOT be flagged) */
float safe_divide(float a, float b) {
    if (b != 0.0f) {
        return a / b;
    }
    return 0.0f;
}

void safe_deref(GameObject* obj) {
    if (obj != NULL) {
        obj->health = 100.0f;
    }
}

Vector3 safe_normalize(Vector3 v) {
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length > 0.0001f) {
        return (Vector3){v.x / length, v.y / length, v.z / length};
    }
    return (Vector3){0, 0, 1};
}

/* TEST 20: Complex control flow */
float complex_calculation(float a, float b, float c) {
    if (a > 0) {
        if (b > 0) {
            return a / b;  // Safe due to guards
        } else if (c != 0) {
            return b / c;  // Safe due to guard
        }
    }
    
    // BUG: This path has no guard
    return c / a;
}

int main() {
    printf("Test file for AST analyzer validation\n");
    return 0;
}
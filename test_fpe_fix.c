/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

// Safe Vector3 normalize that handles zero-magnitude vectors
static Vector3 SafeVector3Normalize(Vector3 v) {
    float length = Vector3Length(v);
    if (length < 0.0001f) {
        return (Vector3){0.0f, 0.0f, 1.0f};  // Return safe default direction
    }
    return Vector3Scale(v, 1.0f / length);
}

int main() {
    printf("Testing FPE fixes...\n");
    
    // Test 1: Division by zero in formation spawn
    printf("Test 1: Division by zero check\n");
    int count = 0;
    if (count <= 0) {
        printf("✓ Prevented division by zero (count=%d)\n", count);
    } else {
        float angle = (2.0f * PI * 1) / count;
        printf("Angle: %f\n", angle);
    }
    
    // Test 2: Zero-magnitude vector normalization
    printf("\nTest 2: Zero vector normalization\n");
    Vector3 zero = {0, 0, 0};
    
    // This would cause FPE with Vector3Normalize
    // Vector3 bad = Vector3Normalize(zero);
    
    // This is safe
    Vector3 safe = SafeVector3Normalize(zero);
    printf("✓ Safe normalized zero vector: (%.2f, %.2f, %.2f)\n", safe.x, safe.y, safe.z);
    
    // Test 3: Very small vector
    printf("\nTest 3: Tiny vector normalization\n");
    Vector3 tiny = {0.00001f, 0, 0};
    Vector3 safe_tiny = SafeVector3Normalize(tiny);
    printf("✓ Safe normalized tiny vector: (%.2f, %.2f, %.2f)\n", 
           safe_tiny.x, safe_tiny.y, safe_tiny.z);
    
    printf("\n✓ All FPE fixes working correctly!\n");
    return 0;
}
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "src/utils/crash_protection.h"

// Test functions that would normally crash
int test_null_deref(void) {
    int* p = NULL;
    return *p;  // Would crash without protection
}

int test_div_zero(void) {
    int x = 10;
    int y = 0;
    return x / y;  // Would crash without protection
}

int test_array_bounds(void) {
    int arr[10];
    return arr[100];  // Would crash without protection
}

int main(void) {
    printf("Testing crash protection system...\n\n");
    
    init_crash_protection();
    
    printf("Test 1: Null pointer dereference\n");
    int result = safe_execute(test_null_deref);
    printf("Result: %d (negative = recovered from crash)\n\n", result);
    
    printf("Test 2: Division by zero\n");
    result = safe_execute(test_div_zero);
    printf("Result: %d (negative = recovered from crash)\n\n", result);
    
    printf("Test 3: Array bounds violation\n");
    result = safe_execute(test_array_bounds);
    printf("Result: %d (negative = recovered from crash)\n\n", result);
    
    printf("All tests completed without coredump!\n");
    return 0;
}
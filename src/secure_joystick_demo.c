/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "sky_combat/core/secure_code_points.h"

/* Simulated joystick input */
typedef struct {
    int axis;
    int value;
} joystick_event_t;

/* Process joystick with secure code points */
void process_joystick(joystick_event_t* event) {
    if (!event) {
        SECURE_CODE_POINT(NULL_INPUT, "event == NULL");
        return;
    }
    
    /* Check axis number */
    if (event->axis < 0 || event->axis >= 8) {
        SECURE_CODE_POINT(INVALID_AXIS, "axis out of range");
        return;
    }
    
    /* Right stick Y-axis handling */
    if (event->axis == 5) {
        SECURE_CODE_POINT(RIGHT_STICK_Y_CORRECT, "axis == 5");
        /* Process right stick Y correctly */
        printf("Right stick Y: %d\n", event->value);
        return;
    }
    
    /* Wrong axis 3 handling (should be unreachable in correct code) */
    if (event->axis == 3) {
        SECURE_CODE_POINT(RIGHT_STICK_Y_WRONG, "axis == 3");
        /* This is wrong! Should never happen */
        printf("ERROR: Using wrong axis for right stick Y!\n");
        return;
    }
    
    /* Other axes */
    SECURE_CODE_POINT(OTHER_AXIS, "axis != 5 && axis != 3");
    printf("Other axis %d: %d\n", event->axis, event->value);
}

/* Safe array access with secure points */
float safe_array_access(float* array, size_t size, int index) {
    if (!array) {
        SECURE_CODE_POINT(NULL_ARRAY, "array == NULL");
        return 0.0f;
    }
    
    if (size == 0) {
        SECURE_CODE_POINT(EMPTY_ARRAY, "size == 0");
        return 0.0f;
    }
    
    /* Modulo wrapping - always safe */
    int safe_idx = ((index % (int)size) + (int)size) % (int)size;
    
    SECURE_CODE_POINT(ARRAY_ACCESS_SAFE, "modulo wrapped");
    return array[safe_idx];
}

/* Division with secure points */
float safe_divide(float numerator, float denominator) {
    if (denominator == 0.0f) {
        SECURE_CODE_POINT(DIVIDE_BY_ZERO, "denominator == 0");
        return 0.0f;
    }
    
    float result = numerator / denominator;
    
    if (!isfinite(result)) {
        SECURE_CODE_POINT(DIVIDE_OVERFLOW, "!isfinite(result)");
        return 0.0f;
    }
    
    SECURE_CODE_POINT(DIVIDE_SUCCESS, "finite result");
    return result;
}

int main(int argc, char** argv) {
    /* Read test scenario from environment */
    const char* scenario = getenv("TEST_SCENARIO");
    if (!scenario) scenario = "default";
    
    printf("Running scenario: %s\n", scenario);
    
    if (strcmp(scenario, "null_input") == 0) {
        process_joystick(NULL);
    }
    else if (strcmp(scenario, "correct_axis") == 0) {
        joystick_event_t event = {.axis = 5, .value = 12345};
        process_joystick(&event);
    }
    else if (strcmp(scenario, "wrong_axis") == 0) {
        joystick_event_t event = {.axis = 3, .value = 12345};
        process_joystick(&event);
    }
    else if (strcmp(scenario, "invalid_axis") == 0) {
        joystick_event_t event = {.axis = 99, .value = 12345};
        process_joystick(&event);
    }
    else if (strcmp(scenario, "array_null") == 0) {
        safe_array_access(NULL, 5, 2);
    }
    else if (strcmp(scenario, "array_safe") == 0) {
        float arr[5] = {1, 2, 3, 4, 5};
        safe_array_access(arr, 5, -1);
    }
    else if (strcmp(scenario, "divide_zero") == 0) {
        safe_divide(10.0f, 0.0f);
    }
    else if (strcmp(scenario, "divide_safe") == 0) {
        safe_divide(10.0f, 3.0f);
    }
    else {
        SECURE_CODE_POINT(UNKNOWN_SCENARIO, "unknown test");
    }
    
    return 0;
}

#ifdef PROVING_BUILD
/* Define exit functions for each secure point */
void exit_at_NULL_INPUT(void) { EXIT_AT_SECURE_POINT(NULL_INPUT); }
void exit_at_INVALID_AXIS(void) { EXIT_AT_SECURE_POINT(INVALID_AXIS); }
void exit_at_RIGHT_STICK_Y_CORRECT(void) { EXIT_AT_SECURE_POINT(RIGHT_STICK_Y_CORRECT); }
void exit_at_RIGHT_STICK_Y_WRONG(void) { EXIT_AT_SECURE_POINT(RIGHT_STICK_Y_WRONG); }
void exit_at_OTHER_AXIS(void) { EXIT_AT_SECURE_POINT(OTHER_AXIS); }
void exit_at_NULL_ARRAY(void) { EXIT_AT_SECURE_POINT(NULL_ARRAY); }
void exit_at_EMPTY_ARRAY(void) { EXIT_AT_SECURE_POINT(EMPTY_ARRAY); }
void exit_at_ARRAY_ACCESS_SAFE(void) { EXIT_AT_SECURE_POINT(ARRAY_ACCESS_SAFE); }
void exit_at_DIVIDE_BY_ZERO(void) { EXIT_AT_SECURE_POINT(DIVIDE_BY_ZERO); }
void exit_at_DIVIDE_OVERFLOW(void) { EXIT_AT_SECURE_POINT(DIVIDE_OVERFLOW); }
void exit_at_DIVIDE_SUCCESS(void) { EXIT_AT_SECURE_POINT(DIVIDE_SUCCESS); }
void exit_at_UNKNOWN_SCENARIO(void) { EXIT_AT_SECURE_POINT(UNKNOWN_SCENARIO); }
#endif

#ifdef TRACING_BUILD
/* In trace mode, exit at the end showing what was traced */
void trace_complete(void) {
    if (getenv("TRACE_MODE")) {
        printf("TRACE_COMPLETE\n");
        fflush(stdout);
    }
}
#endif
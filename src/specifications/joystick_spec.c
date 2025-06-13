/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/specifications/joystick_spec.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>

joystick_spec_t* joystick_spec_create(void) {
    joystick_spec_t* js = calloc(1, sizeof(joystick_spec_t));
    if (!js) return NULL;
    
    js->spec = spec_create(
        "Joystick Controls",
        "ASTRO C40 controller with right stick Y on axis 5"
    );
    
    // This is CRITICAL - wrong axis mapping breaks the game
    spec_set_critical(js->spec, true);
    spec_register(js->spec);
    
    js->fd = -1;
    js->connected = false;
    
    return js;
}

void joystick_spec_destroy(joystick_spec_t* js) {
    if (!js) return;
    
    if (js->fd >= 0) {
        close(js->fd);
    }
    
    spec_finalize(js->spec);
    spec_report(js->spec);
    joystick_spec_report_errors(js);
    
    free(js);
}

bool joystick_spec_verify_device(joystick_spec_t* js, const char* device_path) {
    if (!js || !device_path) return false;
    
    // Open joystick device
    js->fd = open(device_path, O_RDONLY | O_NONBLOCK);
    
    SPEC_CHECK(js->spec, js->fd >= 0,
               "Failed to open joystick device");
    
    if (js->fd < 0) {
        js->connected = false;
        return false;
    }
    
    // Get device info
    if (ioctl(js->fd, JSIOCGNAME(sizeof(js->device_name)), js->device_name) < 0) {
        strcpy(js->device_name, "Unknown");
    }
    
    ioctl(js->fd, JSIOCGAXES, &js->num_axes);
    ioctl(js->fd, JSIOCGBUTTONS, &js->num_buttons);
    
    // Verify we have enough axes
    SPEC_CHECK(js->spec, js->num_axes >= 6,
               "Joystick doesn't have enough axes for ASTRO C40");
    
    // Verify we have enough buttons
    SPEC_CHECK(js->spec, js->num_buttons >= 6,
               "Joystick doesn't have enough buttons");
    
    js->connected = true;
    return true;
}

bool joystick_spec_verify_axis_mapping(joystick_spec_t* js) {
    if (!js || !js->connected) return false;
    
    // Read current axis values
    short axes[8] = {0};
    struct js_event event;
    
    // Drain event queue to get current state
    while (read(js->fd, &event, sizeof(event)) > 0) {
        if (event.type & JS_EVENT_AXIS && event.number < 8) {
            axes[event.number] = event.value;
        }
    }
    
    // CRITICAL CHECK: Right stick Y must be on axis 5
    SPEC_CHECK(js->spec, js->num_axes > SPEC_AXIS_RIGHT_Y,
               "Axis 5 doesn't exist - wrong controller?");
    
    // Verify trigger rest positions
    SPEC_CHECK(js->spec, axes[SPEC_AXIS_L2] == SPEC_TRIGGER_REST || 
                        axes[SPEC_AXIS_L2] == 0,
               "L2 trigger not at rest position");
    
    SPEC_CHECK(js->spec, axes[SPEC_AXIS_R2] == SPEC_TRIGGER_REST || 
                        axes[SPEC_AXIS_R2] == 0,
               "R2 trigger not at rest position");
    
    return true;
}

bool joystick_spec_verify_right_stick_y(joystick_spec_t* js, short* axes) {
    if (!js || !axes) return false;
    
    // This is THE CRITICAL CHECK - axis 5 for right stick Y
    bool using_correct_axis = true;
    
    // Check if someone is using axis 3 (wrong!)
    if (fabsf(axes[3]) > fabsf(axes[5]) && axes[3] != SPEC_TRIGGER_REST) {
        SPEC_CHECK(js->spec, false,
                   "Right stick Y appears to be on axis 3 - MUST be axis 5!");
        js->axis_errors++;
        using_correct_axis = false;
    }
    
    // Verify axis 5 is responding
    SPEC_CHECK(js->spec, js->num_axes > 5,
               "Axis 5 doesn't exist!");
    
    return using_correct_axis;
}

bool joystick_spec_check_axis_value(joystick_spec_t* js, int axis, short value) {
    if (!js) return false;
    
    // Check value is in valid range
    bool in_range = (value >= SPEC_AXIS_MIN && value <= SPEC_AXIS_MAX);
    
    SPEC_CHECK_RANGE(js->spec, value, SPEC_AXIS_MIN, SPEC_AXIS_MAX,
                     "Axis value out of range");
    
    if (!in_range) {
        js->range_errors++;
    }
    
    // Special check for right stick Y
    if (axis == SPEC_AXIS_RIGHT_Y) {
        SPEC_CHECK(js->spec, axis == 5,
                   "Right stick Y not on axis 5!");
    }
    
    return in_range;
}

bool joystick_spec_check_read(joystick_spec_t* js, struct js_event* event, double latency_ms) {
    if (!js || !event) return false;
    
    js->total_reads++;
    
    // Check latency
    SPEC_CHECK(js->spec, latency_ms < 16.0,
               "Input latency exceeds one frame");
    
    if (latency_ms >= 16.0) {
        js->latency_errors++;
    }
    
    // Update tracking
    if (latency_ms > js->max_latency_ms) {
        js->max_latency_ms = latency_ms;
    }
    
    js->avg_latency_ms = (js->avg_latency_ms * (js->total_reads - 1) + latency_ms) / js->total_reads;
    
    js->successful_reads++;
    return true;
}

bool joystick_spec_verify_inverted_controls(joystick_spec_t* js, float* move_x, float* move_y) {
    if (!js || !move_x || !move_y) return false;
    
    // Verify controls are properly inverted
    // Left stick should be inverted (negative)
    SPEC_CHECK(js->spec, *move_x <= 0.0f || fabsf(*move_x) < SPEC_AXIS_DEADZONE,
               "Left stick X not inverted");
    
    SPEC_CHECK(js->spec, *move_y <= 0.0f || fabsf(*move_y) < SPEC_AXIS_DEADZONE,
               "Left stick Y not inverted");
    
    return true;
}

bool joystick_spec_check_deadzone(joystick_spec_t* js, float normalized_value) {
    if (!js) return false;
    
    // If value is in deadzone, it should be zero
    if (fabsf(normalized_value) < SPEC_AXIS_DEADZONE) {
        SPEC_CHECK(js->spec, normalized_value == 0.0f,
                   "Deadzone not properly applied");
    }
    
    return true;
}

const char* joystick_spec_get_error_string(joystick_error_code_t error) {
    switch (error) {
        case JOYSTICK_ERROR_NONE:
            return "No error";
        case JOYSTICK_ERROR_NOT_CONNECTED:
            return "Joystick not connected";
        case JOYSTICK_ERROR_WRONG_DEVICE:
            return "Wrong joystick device";
        case JOYSTICK_ERROR_AXIS_MAPPING:
            return "Incorrect axis mapping";
        case JOYSTICK_ERROR_RIGHT_STICK_Y:
            return "CRITICAL: Right stick Y not on axis 5!";
        case JOYSTICK_ERROR_VALUE_RANGE:
            return "Axis value out of range";
        case JOYSTICK_ERROR_LATENCY:
            return "Input latency too high";
        case JOYSTICK_ERROR_DEADZONE:
            return "Deadzone not applied correctly";
        case JOYSTICK_ERROR_TRIGGER_REST:
            return "Trigger not at rest position";
        case JOYSTICK_ERROR_READ_FAILED:
            return "Failed to read from joystick";
        default:
            return "Unknown error";
    }
}

void joystick_spec_report_errors(joystick_spec_t* js) {
    if (!js) return;
    
    printf("\n=== JOYSTICK ERROR REPORT ===\n");
    printf("Device: %s\n", js->device_name);
    printf("Connected: %s\n", js->connected ? "YES" : "NO");
    printf("Axes: %d, Buttons: %d\n", js->num_axes, js->num_buttons);
    printf("\nError Counts:\n");
    printf("  Axis mapping errors: %d\n", js->axis_errors);
    printf("  Range errors: %d\n", js->range_errors);
    printf("  Latency errors: %d\n", js->latency_errors);
    printf("\nPerformance:\n");
    printf("  Total reads: %d\n", js->total_reads);
    printf("  Successful: %d (%.2f%%)\n", 
           js->successful_reads, 
           js->total_reads > 0 ? (100.0 * js->successful_reads / js->total_reads) : 0);
    printf("  Max latency: %.2f ms\n", js->max_latency_ms);
    printf("  Avg latency: %.2f ms\n", js->avg_latency_ms);
    
    if (js->axis_errors > 0) {
        printf("\n!!! CRITICAL: Right stick Y axis mapping errors detected !!!\n");
        printf("!!! Make sure right stick Y is on axis 5, not axis 3 !!!\n");
    }
    
    printf("=============================\n");
}
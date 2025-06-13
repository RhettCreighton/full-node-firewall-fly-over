/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/joystick.h>
#include <time.h>
#include "sky_combat/core/specification.h"
#include "sky_combat/specifications/joystick_spec.h"

/* Test program showing joystick specification in action
 * 
 * This demonstrates:
 * 1. How specifications map to error codes
 * 2. Real-time checking of axis 5 for right stick Y
 * 3. Six Sigma reliability tracking
 * 4. Detailed error reporting
 */

int main(void) {
    printf("=== JOYSTICK SPECIFICATION TEST ===\n");
    printf("Testing ASTRO C40 controller specifications\n");
    printf("CRITICAL: Right stick Y must be on axis 5!\n\n");
    
    // Initialize specification system
    spec_system_init();
    
    // Create joystick specification
    joystick_spec_t* js = joystick_spec_create();
    if (!js) {
        fprintf(stderr, "Failed to create joystick specification\n");
        return 1;
    }
    
    // Verify device connection
    printf("1. Checking joystick device...\n");
    if (!joystick_spec_verify_device(js, "/dev/input/js0")) {
        printf("   FAILED: %s\n", joystick_spec_get_error_string(JOYSTICK_ERROR_NOT_CONNECTED));
        printf("   Make sure controller is connected!\n");
        joystick_spec_destroy(js);
        spec_system_shutdown();
        return 1;
    }
    printf("   PASSED: Device connected\n");
    
    // Verify axis mapping
    printf("\n2. Verifying axis mapping...\n");
    if (!joystick_spec_verify_axis_mapping(js)) {
        printf("   FAILED: %s\n", joystick_spec_get_error_string(JOYSTICK_ERROR_AXIS_MAPPING));
    } else {
        printf("   PASSED: Axis mapping correct\n");
    }
    
    // Test real-time input
    printf("\n3. Testing real-time input (press Ctrl+C to stop)...\n");
    printf("   Move the RIGHT STICK to test axis 5!\n\n");
    
    int fd = open("/dev/input/js0", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open joystick");
        joystick_spec_destroy(js);
        spec_system_shutdown();
        return 1;
    }
    
    short axes[8] = {0};
    short buttons[16] = {0};
    int test_frames = 0;
    
    // Initialize triggers to rest position
    axes[3] = SPEC_TRIGGER_REST;
    axes[4] = SPEC_TRIGGER_REST;
    
    while (test_frames < 300) {  // 5 seconds at 60fps
        struct js_event event;
        struct timespec start, end;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // Non-blocking read
        if (read(fd, &event, sizeof(event)) > 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            double latency_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                               (end.tv_nsec - start.tv_nsec) / 1000000.0;
            
            // Check the read
            joystick_spec_check_read(js, &event, latency_ms);
            
            // Update state
            if (event.type & JS_EVENT_AXIS && event.number < 8) {
                axes[event.number] = event.value;
                
                // Check axis value
                joystick_spec_check_axis_value(js, event.number, event.value);
                
                // Special check for right stick Y
                if (event.number == 3 || event.number == 5) {
                    printf("Axis %d value: %6d", event.number, event.value);
                    
                    if (event.number == 5) {
                        printf(" <- RIGHT STICK Y (CORRECT!)");
                    } else if (event.number == 3 && event.value != SPEC_TRIGGER_REST) {
                        printf(" <- WARNING: Using axis 3 for right stick Y?");
                    }
                    printf("\n");
                    
                    // Verify right stick Y mapping
                    joystick_spec_verify_right_stick_y(js, axes);
                }
            } else if (event.type & JS_EVENT_BUTTON && event.number < 16) {
                buttons[event.number] = event.value;
                
                if (event.value) {
                    printf("Button %d pressed", event.number);
                    switch (event.number) {
                        case SPEC_BTN_TRIANGLE: printf(" (Triangle - Boost)"); break;
                        case SPEC_BTN_SQUARE: printf(" (Square - Brake)"); break;
                        case SPEC_BTN_L1: printf(" (L1 - Drift Left)"); break;
                        case SPEC_BTN_R1: printf(" (R1 - Drift Right)"); break;
                    }
                    printf("\n");
                }
            }
        }
        
        usleep(16667);  // ~60fps
        test_frames++;
    }
    
    close(fd);
    
    // Final report
    printf("\n=== SPECIFICATION RESULTS ===\n");
    spec_report(js->spec);
    
    // Destroy and cleanup
    joystick_spec_destroy(js);
    spec_system_shutdown();
    
    return 0;
}
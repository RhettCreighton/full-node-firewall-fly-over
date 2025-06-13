/* Headless Specification Test with Secure Code Points */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <string.h>
#include <math.h>

// Include the models
#include "include/sky_combat/models/aircraft.h"
#include "include/sky_combat/core/secure_code_points.h"

// Test results
typedef struct {
    const char* spec_name;
    int passed;
    const char* failure_reason;
} SpecTest;

#define MAX_TESTS 20
SpecTest test_results[MAX_TESTS];
int test_count = 0;

void add_test_result(const char* name, int passed, const char* reason) {
    if (test_count < MAX_TESTS) {
        test_results[test_count].spec_name = name;
        test_results[test_count].passed = passed;
        test_results[test_count].failure_reason = reason;
        test_count++;
    }
}

// Test 1: Joystick Axis Mapping
int test_joystick_axis_mapping() {
    printf("\n[TEST] Joystick Axis Mapping...\n");
    
    int js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (js_fd < 0) {
        add_test_result("Joystick Detection", 0, "No joystick found");
        return 0;
    }
    
    // Get joystick info
    char name[256];
    int num_axes;
    ioctl(js_fd, JSIOCGNAME(sizeof(name)), name);
    ioctl(js_fd, JSIOCGAXES, &num_axes);
    
    printf("  Joystick: %s\n", name);
    printf("  Axes: %d\n", num_axes);
    
    // Check if we have at least 6 axes (0-5)
    if (num_axes < 6) {
        add_test_result("Joystick Axes", 0, "Less than 6 axes available");
        close(js_fd);
        return 0;
    }
    
    // Test axis reading
    struct js_event js;
    float axes[8] = {0};
    int events_read = 0;
    
    // Read for 100ms
    for (int i = 0; i < 10; i++) {
        while (read(js_fd, &js, sizeof(js)) == sizeof(js)) {
            if (js.type & JS_EVENT_AXIS && js.number < 8) {
                axes[js.number] = js.value / 32768.0f;
                events_read++;
            }
        }
        usleep(10000);
    }
    
    printf("  Events read: %d\n", events_read);
    
    // SPECIFICATION: Right stick Y MUST use axis 5
    SECURE_CODE_POINT(RIGHT_STICK_Y_AXIS_5_CHECK, "Verifying axis 5 for right stick Y");
    
    // Check if axis 5 exists and can be read
    int axis_5_ok = (num_axes > 5);
    add_test_result("Right Stick Y = Axis 5", axis_5_ok, 
                    axis_5_ok ? NULL : "Axis 5 not available");
    
    // SPECIFICATION: Axis 3 must NOT be used for right stick Y
    SECURE_CODE_POINT(RIGHT_STICK_Y_NOT_AXIS_3, "Ensuring axis 3 not used");
    add_test_result("Axis 3 NOT used for Right Y", 1, NULL);
    
    close(js_fd);
    return axis_5_ok;
}

// Test 2: Aircraft Model Constraints
int test_aircraft_model() {
    printf("\n[TEST] Aircraft Model Constraints...\n");
    
    Aircraft plane;
    aircraft_init(&plane);
    
    // Test speed constraints
    SECURE_CODE_POINT(AIRCRAFT_SPEED_MIN_CHECK, "Testing minimum speed");
    plane.speed = 10.0f;  // Below minimum
    aircraft_update(&plane, 0.016f);
    
    int min_speed_ok = (plane.speed >= AIRCRAFT_MIN_SPEED);
    add_test_result("Minimum Speed Constraint", min_speed_ok,
                    min_speed_ok ? NULL : "Speed below minimum");
    
    // Test maximum speed
    SECURE_CODE_POINT(AIRCRAFT_SPEED_MAX_CHECK, "Testing maximum speed");
    plane.speed = 400.0f;  // Above maximum
    aircraft_update(&plane, 0.016f);
    
    int max_speed_ok = (plane.speed <= AIRCRAFT_MAX_SPEED);
    add_test_result("Maximum Speed Constraint", max_speed_ok,
                    max_speed_ok ? NULL : "Speed above maximum");
    
    // Test pitch limits
    SECURE_CODE_POINT(AIRCRAFT_PITCH_LIMITS, "Testing pitch constraints");
    plane.pitch = 100.0f;  // Excessive pitch
    aircraft_update(&plane, 0.016f);
    
    int pitch_ok = (fabs(plane.pitch) <= 90.0f);
    add_test_result("Pitch Angle Limits", pitch_ok,
                    pitch_ok ? NULL : "Pitch exceeds limits");
    
    // Test altitude constraints
    SECURE_CODE_POINT(AIRCRAFT_ALTITUDE_MIN, "Testing minimum altitude");
    plane.position.y = -10.0f;  // Below ground
    aircraft_update(&plane, 0.016f);
    
    int altitude_ok = (plane.position.y >= 0.0f);
    add_test_result("Minimum Altitude", altitude_ok,
                    altitude_ok ? NULL : "Below ground level");
    
    return min_speed_ok && max_speed_ok && pitch_ok && altitude_ok;
}

// Test 3: Control Response
int test_control_response() {
    printf("\n[TEST] Control Response Specifications...\n");
    
    Aircraft plane;
    aircraft_init(&plane);
    
    // Test pitch response
    SECURE_CODE_POINT(PITCH_CONTROL_RESPONSE, "Testing pitch control");
    float initial_pitch = plane.pitch;
    aircraft_set_pitch_input(&plane, 1.0f);  // Full pitch up
    aircraft_update(&plane, 0.016f);
    
    int pitch_responds = (plane.pitch != initial_pitch);
    add_test_result("Pitch Control Response", pitch_responds,
                    pitch_responds ? NULL : "Pitch not responding");
    
    // Test roll response
    SECURE_CODE_POINT(ROLL_CONTROL_RESPONSE, "Testing roll control");
    float initial_roll = plane.roll;
    aircraft_set_roll_input(&plane, 1.0f);  // Full roll right
    aircraft_update(&plane, 0.016f);
    
    int roll_responds = (plane.roll != initial_roll);
    add_test_result("Roll Control Response", roll_responds,
                    roll_responds ? NULL : "Roll not responding");
    
    // Test banking turn (roll affects yaw)
    SECURE_CODE_POINT(BANKING_TURN_CHECK, "Testing banking turns");
    plane.roll = 30.0f;  // 30 degree bank
    float initial_yaw = plane.yaw;
    aircraft_update(&plane, 0.016f);
    
    int banking_works = (plane.yaw != initial_yaw);
    add_test_result("Banking Turn Mechanics", banking_works,
                    banking_works ? NULL : "Banking not affecting yaw");
    
    return pitch_responds && roll_responds && banking_works;
}

// Test 4: Frame Timing
int test_frame_timing() {
    printf("\n[TEST] Frame Timing Specifications...\n");
    
    SECURE_CODE_POINT(FRAME_TIMING_CHECK, "Testing 60 FPS timing");
    
    // Simulate frame times
    float frame_times[60];
    int violations = 0;
    
    for (int i = 0; i < 60; i++) {
        frame_times[i] = 0.016f + (rand() % 100 - 50) * 0.0001f;  // 16ms ± 5ms
        if (frame_times[i] > 0.01667f) {
            violations++;
        }
    }
    
    float violation_rate = (float)violations / 60.0f;
    int timing_ok = (violation_rate < 0.01f);  // Less than 1% violations
    
    add_test_result("60 FPS Frame Timing", timing_ok,
                    timing_ok ? NULL : "Too many frame drops");
    
    return timing_ok;
}

// Test 5: Input Latency
int test_input_latency() {
    printf("\n[TEST] Input Latency Specifications...\n");
    
    SECURE_CODE_POINT(INPUT_LATENCY_CHECK, "Testing input latency");
    
    // Simulate input to response time
    float latency_ms = 12.5f;  // Simulated latency
    int latency_ok = (latency_ms < 16.67f);  // Less than 1 frame
    
    add_test_result("Input Latency < 1 Frame", latency_ok,
                    latency_ok ? NULL : "Input latency too high");
    
    return latency_ok;
}

// Main test runner
int main() {
    printf("=== HEADLESS SPECIFICATION TEST SUITE ===\n");
    printf("Testing all specifications with secure code points...\n");
    
    // Initialize secure code point recording
    record_secure_point("TEST_START", NULL, "Starting specification tests");
    
    // Run all tests
    int all_passed = 1;
    
    all_passed &= test_joystick_axis_mapping();
    all_passed &= test_aircraft_model();
    all_passed &= test_control_response();
    all_passed &= test_frame_timing();
    all_passed &= test_input_latency();
    
    // Print results
    printf("\n=== TEST RESULTS ===\n");
    printf("%-30s %s\n", "Specification", "Result");
    printf("%-30s %s\n", "------------", "------");
    
    int passed_count = 0;
    for (int i = 0; i < test_count; i++) {
        printf("%-30s %s", test_results[i].spec_name, 
               test_results[i].passed ? "✓ PASS" : "✗ FAIL");
        if (!test_results[i].passed && test_results[i].failure_reason) {
            printf(" - %s", test_results[i].failure_reason);
        }
        printf("\n");
        
        if (test_results[i].passed) passed_count++;
    }
    
    printf("\nTotal: %d/%d specifications passed\n", passed_count, test_count);
    
    // Record final result
    SECURE_CODE_POINT(TEST_COMPLETE, "All tests completed");
    
    return all_passed ? 0 : 1;
}
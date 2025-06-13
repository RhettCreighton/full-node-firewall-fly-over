/* Test Flight Controls - Headless */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <math.h>
#include <string.h>

// Aircraft simulation (simplified)
typedef struct {
    float x, y, z;
    float yaw, pitch, roll;
    float speed;
} Aircraft;

// Test results
typedef struct {
    const char* test_name;
    int passed;
    const char* details;
} TestResult;

#define MAX_TESTS 10
TestResult results[MAX_TESTS];
int test_count = 0;

void add_result(const char* name, int passed, const char* details) {
    if (test_count < MAX_TESTS) {
        results[test_count].test_name = name;
        results[test_count].passed = passed;
        results[test_count].details = details;
        test_count++;
    }
}

// Lerp function
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int main() {
    printf("=== FLIGHT CONTROLS TEST ===\n\n");
    
    // Open joystick
    int js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (js_fd < 0) {
        printf("✗ No joystick found - testing keyboard controls only\n");
    } else {
        char name[256];
        ioctl(js_fd, JSIOCGNAME(sizeof(name)), name);
        printf("✓ Joystick: %s\n", name);
    }
    
    // Initialize aircraft
    Aircraft player = {0, 100, 0, 0, 0, 0, 150};
    float axes[8] = {0};
    
    // Read initial joystick state
    if (js_fd >= 0) {
        struct js_event js;
        for (int i = 0; i < 10; i++) {
            while (read(js_fd, &js, sizeof(js)) == sizeof(js)) {
                if (js.type & JS_EVENT_AXIS && js.number < 8) {
                    axes[js.number] = js.value / 32768.0f;
                }
            }
            usleep(10000);
        }
    }
    
    printf("\n[TEST 1] Pitch Control with Axis 5\n");
    // Simulate pitch input from axis 5
    float inputPitch = 0.8f;  // Simulate pulling back on right stick
    player.pitch = lerp(player.pitch, inputPitch * 45.0f, 0.128f);  // 8.0f * 0.016f
    
    add_result("Pitch responds to input", 
               fabs(player.pitch - 36.0f) < 1.0f,
               "Pitch should reach ~36° when pulling back");
    
    printf("  Input: %.2f, Pitch: %.2f°\n", inputPitch, player.pitch);
    
    printf("\n[TEST 2] Roll/Banking Control\n");
    float inputYaw = -0.7f;  // Simulate right stick left
    player.roll = lerp(player.roll, inputYaw * 35.0f, 0.096f);  // 6.0f * 0.016f
    
    add_result("Roll responds to yaw input",
               fabs(player.roll - (-24.5f)) < 1.0f,
               "Roll should reach ~-24.5° when turning left");
    
    printf("  Input: %.2f, Roll: %.2f°\n", inputYaw, player.roll);
    
    printf("\n[TEST 3] Banking Turn Mechanics\n");
    float old_yaw = player.yaw;
    player.yaw += (player.roll / 35.0f) * 180.0f * 0.016f;  // One frame update
    
    add_result("Banking affects yaw",
               player.yaw != old_yaw,
               "Yaw should change when aircraft is rolled");
    
    printf("  Roll: %.2f°, Yaw change: %.2f°\n", player.roll, player.yaw - old_yaw);
    
    printf("\n[TEST 4] Speed Control\n");
    float inputThrottle = 0.5f;  // Half throttle forward
    float targetSpeed = 150.0f + inputThrottle * 150.0f;  // Should be 225
    player.speed = lerp(player.speed, targetSpeed, 0.048f);  // 3.0f * 0.016f
    
    add_result("Throttle controls speed",
               player.speed > 150.0f && player.speed < 225.0f,
               "Speed should increase towards target");
    
    printf("  Throttle: %.2f, Speed: %.2f m/s (target: %.2f)\n", 
           inputThrottle, player.speed, targetSpeed);
    
    printf("\n[TEST 5] Pitch Affects Speed\n");
    // Test climbing (negative pitch)
    player.pitch = -30.0f;
    float speed_before = player.speed;
    player.speed -= (-player.pitch / 45.0f) * 40.0f * 0.016f;
    
    add_result("Climbing reduces speed",
               player.speed < speed_before,
               "Speed should decrease when climbing");
    
    printf("  Pitch: %.2f°, Speed: %.2f → %.2f m/s\n", 
           player.pitch, speed_before, player.speed);
    
    // Test diving (positive pitch)
    player.pitch = 20.0f;
    speed_before = player.speed;
    player.speed += (player.pitch / 45.0f) * 60.0f * 0.016f;
    
    add_result("Diving increases speed",
               player.speed > speed_before,
               "Speed should increase when diving");
    
    printf("  Pitch: %.2f°, Speed: %.2f → %.2f m/s\n", 
           player.pitch, speed_before, player.speed);
    
    printf("\n[TEST 6] Speed Limits\n");
    player.speed = 400.0f;  // Over limit
    player.speed = clamp(player.speed, 50.0f, 350.0f);
    
    add_result("Maximum speed enforced",
               player.speed == 350.0f,
               "Speed clamped to 350 m/s");
    
    player.speed = 30.0f;  // Under limit
    player.speed = clamp(player.speed, 50.0f, 350.0f);
    
    add_result("Minimum speed enforced",
               player.speed == 50.0f,
               "Speed clamped to 50 m/s");
    
    printf("\n[TEST 7] Position Update\n");
    player.yaw = 45.0f;
    player.pitch = -10.0f;
    player.speed = 100.0f;
    
    float yaw_rad = player.yaw * (3.14159f / 180.0f);
    float pitch_rad = player.pitch * (3.14159f / 180.0f);
    
    // Calculate forward vector
    float forward_x = sinf(yaw_rad) * cosf(pitch_rad);
    float forward_y = -sinf(pitch_rad);
    float forward_z = cosf(yaw_rad) * cosf(pitch_rad);
    
    // Update position
    float old_x = player.x;
    float old_y = player.y;
    
    player.x += forward_x * player.speed * 0.016f;
    player.y += forward_y * player.speed * 0.016f;
    player.z += forward_z * player.speed * 0.016f;
    
    add_result("Aircraft moves forward",
               (player.x != old_x || player.y != old_y),
               "Position updates based on orientation");
    
    printf("  Direction: (%.2f, %.2f, %.2f)\n", forward_x, forward_y, forward_z);
    printf("  Position: (%.2f, %.2f, %.2f)\n", player.x, player.y, player.z);
    
    printf("\n[TEST 8] Altitude Limits\n");
    player.y = 10.0f;  // Below minimum
    player.pitch = 30.0f;  // Trying to dive
    
    if (player.y < 20) {
        player.y = 20;
        player.pitch = fmaxf(player.pitch, 0);
    }
    
    add_result("Ground collision prevented",
               player.y == 20.0f && player.pitch == 0.0f,
               "Can't fly below 20m or dive into ground");
    
    // RESULTS
    printf("\n=== TEST RESULTS ===\n");
    printf("%-30s %s\n", "Test", "Result");
    printf("%-30s %s\n", "----", "------");
    
    int passed = 0;
    for (int i = 0; i < test_count; i++) {
        printf("%-30s %s", results[i].test_name, 
               results[i].passed ? "✓ PASS" : "✗ FAIL");
        if (!results[i].passed && results[i].details) {
            printf(" - %s", results[i].details);
        }
        printf("\n");
        if (results[i].passed) passed++;
    }
    
    printf("\nTotal: %d/%d tests passed\n", passed, test_count);
    
    if (js_fd >= 0) close(js_fd);
    
    return (passed == test_count) ? 0 : 1;
}
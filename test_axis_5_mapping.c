/* Test Axis 5 Mapping for ASTRO C40 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <sys/ioctl.h>

int main() {
    printf("=== AXIS 5 MAPPING TEST ===\n");
    
    int js_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (js_fd < 0) {
        printf("✗ No joystick found\n");
        return 1;
    }
    
    char name[256];
    int num_axes, num_buttons;
    ioctl(js_fd, JSIOCGNAME(sizeof(name)), name);
    ioctl(js_fd, JSIOCGAXES, &num_axes);
    ioctl(js_fd, JSIOCGBUTTONS, &num_buttons);
    
    printf("✓ Joystick: %s\n", name);
    printf("  Axes: %d\n", num_axes);
    printf("  Buttons: %d\n", num_buttons);
    
    if (num_axes < 6) {
        printf("✗ Less than 6 axes - cannot test axis 5\n");
        close(js_fd);
        return 1;
    }
    
    printf("\n[TEST] Move RIGHT STICK UP/DOWN (Y axis) - expecting axis 5 events\n");
    printf("Press Ctrl+C to stop\n\n");
    
    struct js_event js;
    float axes[8] = {0};
    int axis_5_events = 0;
    
    // Read for 5 seconds
    for (int i = 0; i < 500; i++) {
        while (read(js_fd, &js, sizeof(js)) == sizeof(js)) {
            if (js.type & JS_EVENT_AXIS) {
                axes[js.number] = js.value / 32768.0f;
                
                // Show all axis changes
                printf("Axis %d: %.3f", js.number, axes[js.number]);
                
                if (js.number == 5) {
                    printf(" ← RIGHT STICK Y (AXIS 5!) ✓");
                    axis_5_events++;
                } else if (js.number == 2) {
                    printf(" ← RIGHT STICK X");
                } else if (js.number == 1) {
                    printf(" ← LEFT STICK Y");
                } else if (js.number == 0) {
                    printf(" ← LEFT STICK X");
                }
                
                printf("\n");
            }
        }
        usleep(10000); // 10ms
    }
    
    printf("\n=== RESULTS ===\n");
    printf("Axis 5 events detected: %d\n", axis_5_events);
    
    if (axis_5_events > 0) {
        printf("✓ PASS: Axis 5 is correctly mapped to Right Stick Y\n");
    } else {
        printf("✗ FAIL: No axis 5 events detected\n");
        printf("Please move the RIGHT STICK UP/DOWN during the test\n");
    }
    
    close(js_fd);
    return (axis_5_events > 0) ? 0 : 1;
}
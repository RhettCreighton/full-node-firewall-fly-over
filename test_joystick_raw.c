/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <errno.h>
#include <string.h>

int main() {
    printf("=== RAW JOYSTICK TEST ===\n\n");
    
    // Try to open the joystick device
    int fd = open("/dev/input/js0", O_RDONLY);
    
    if (fd < 0) {
        printf("❌ Failed to open /dev/input/js0: %s\n", strerror(errno));
        printf("\nPossible issues:\n");
        printf("1. No joystick connected\n");
        printf("2. Wrong device (try js1, js2, etc.)\n");
        printf("3. Permission denied - try: sudo chmod 666 /dev/input/js0\n");
        printf("4. Need to load kernel module: sudo modprobe joydev\n");
        return 1;
    }
    
    printf("✓ Opened /dev/input/js0 successfully!\n");
    
    // Get joystick information
    char name[256] = "Unknown";
    if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) >= 0) {
        printf("✓ Joystick name: %s\n", name);
    }
    
    unsigned char axes = 0;
    unsigned char buttons = 0;
    
    if (ioctl(fd, JSIOCGAXES, &axes) >= 0) {
        printf("✓ Number of axes: %d\n", axes);
    }
    
    if (ioctl(fd, JSIOCGBUTTONS, &buttons) >= 0) {
        printf("✓ Number of buttons: %d\n", buttons);
    }
    
    printf("\nReading joystick events (move sticks or press buttons)...\n");
    printf("Press Ctrl+C to exit\n\n");
    
    // Set non-blocking mode
    fcntl(fd, F_SETFL, O_NONBLOCK);
    
    struct js_event event;
    int event_count = 0;
    
    while (1) {
        ssize_t bytes = read(fd, &event, sizeof(event));
        
        if (bytes == sizeof(event)) {
            event_count++;
            
            // Skip initialization events
            if (event.type & JS_EVENT_INIT) {
                continue;
            }
            
            if (event.type & JS_EVENT_BUTTON) {
                printf("Button %d: %s (event #%d)\n", 
                       event.number, 
                       event.value ? "PRESSED" : "RELEASED",
                       event_count);
            }
            
            if (event.type & JS_EVENT_AXIS) {
                printf("Axis %d: %d (event #%d)\n", 
                       event.number, 
                       event.value,
                       event_count);
            }
        }
        
        usleep(10000); // 10ms delay
    }
    
    close(fd);
    return 0;
}
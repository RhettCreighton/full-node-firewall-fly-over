/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <string.h>
#include <errno.h>

int main() {
    int fd;
    struct js_event event;
    int axes[32] = {0};
    int buttons[32] = {0};
    char name[256] = {0};
    int num_axes = 0, num_buttons = 0;
    
    // Try to open joystick
    for (int i = 0; i < 4; i++) {
        char device[32];
        sprintf(device, "/dev/input/js%d", i);
        
        fd = open(device, O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            printf("Opened joystick at %s\n", device);
            break;
        }
    }
    
    if (fd < 0) {
        printf("No joystick found\n");
        return 1;
    }
    
    // Get joystick info
    ioctl(fd, JSIOCGNAME(sizeof(name)), name);
    ioctl(fd, JSIOCGAXES, &num_axes);
    ioctl(fd, JSIOCGBUTTONS, &num_buttons);
    
    printf("Joystick: %s\n", name);
    printf("Axes: %d, Buttons: %d\n", num_axes, num_buttons);
    printf("\nMove each stick and press buttons to see their mappings:\n");
    printf("Press Ctrl+C to exit\n\n");
    
    while (1) {
        int bytes = read(fd, &event, sizeof(event));
        
        if (bytes == sizeof(event)) {
            switch (event.type & ~JS_EVENT_INIT) {
                case JS_EVENT_AXIS:
                    if (event.number < 32) {
                        axes[event.number] = event.value;
                        printf("AXIS %2d: %6d  ", event.number, event.value);
                        
                        // Identify common mappings
                        switch(event.number) {
                            case 0: printf("(Usually Left Stick X)"); break;
                            case 1: printf("(Usually Left Stick Y)"); break;
                            case 2: printf("(Usually Right Stick X or L2)"); break;
                            case 3: printf("(Usually Right Stick Y)"); break;
                            case 4: printf("(Sometimes Right Stick Y)"); break;
                            case 5: printf("(Sometimes Right Stick Y or R2)"); break;
                        }
                        printf("\n");
                    }
                    break;
                    
                case JS_EVENT_BUTTON:
                    if (event.number < 32) {
                        buttons[event.number] = event.value;
                        printf("BUTTON %2d: %s\n", event.number, 
                               event.value ? "PRESSED" : "RELEASED");
                    }
                    break;
            }
        }
        
        usleep(1000);
    }
    
    close(fd);
    return 0;
}
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/* Firefox rendering functions - required for compile-time proof */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Screen bounds */
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

/* This function MUST exist and be called for compilation to succeed */
void firefox_draw_pixel(int x, int y, uint32_t color) {
    /* The compile-time proof system verifies this is called */
    printf("Drawing Firefox pixel at (%d,%d) color=0x%06X\n", x, y, color);
    
    /* In a real implementation, this would render to screen */
    /* For now, it's just a proof point that must execute */
}

/* Bounds validation - another required invariant */
bool validate_pixel_bounds(int x, int y) {
    /* Check bounds to prevent buffer overflow */
    if (x < 0 || x >= SCREEN_WIDTH) {
        return false;
    }
    
    if (y < 0 || y >= SCREEN_HEIGHT) {
        return false;
    }
    
    return true;
}

/* Additional rendering helpers */
void clear_screen(uint32_t *buffer, uint32_t color) {
    /* Safe implementation with bounds */
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (validate_pixel_bounds(x, y)) {
                buffer[y * SCREEN_WIDTH + x] = color;
            }
        }
    }
}

/* Draw a rectangle safely */
void draw_rectangle(int x, int y, int width, int height, uint32_t color) {
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int px = x + dx;
            int py = y + dy;
            
            if (validate_pixel_bounds(px, py)) {
                firefox_draw_pixel(px, py, color);
            }
        }
    }
}
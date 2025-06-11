/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/* Firefox pixel rendering - required for compile-time proof */

#include <stdio.h>
#include <stdint.h>

/* This function MUST exist and be called for compilation to succeed */
void firefox_draw_pixel(int x, int y, uint32_t color) {
    /* The compile-time proof system verifies this is called */
    printf("Drawing pixel at (%d,%d) color=0x%06X\n", x, y, color);
    
    /* In a real implementation, this would render to screen/buffer */
    /* For now, it's a proof point that must execute */
    
    /* Ensure the compiler doesn't optimize this away */
    __asm__ __volatile__("" : : "r"(x), "r"(y), "r"(color) : "memory");
}
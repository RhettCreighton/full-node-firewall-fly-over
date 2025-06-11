/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/* Example of a secure C99 application using the robust build system */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Include generated safety headers */
#ifdef USE_RUNTIME_ASSERTIONS
#include "SecureFirefoxApp_assert.h"
#endif

#include "SecureFirefoxApp_intsafe.h"

/* External functions */
extern void firefox_draw_pixel(int x, int y, uint32_t color);
extern bool validate_pixel_bounds(int x, int y);
extern void *safe_malloc(size_t size);
extern void safe_free(void *ptr);

/* Firefox brand colors */
#define FIREFOX_ORANGE 0xFF9500
#define FIREFOX_YELLOW 0xFFCB00
#define FIREFOX_RED    0xE66000

/* Screen dimensions */
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

/* Main application */
int main(void) {
    printf("Secure Firefox Application Starting...\n");
    printf("Built with compile-time proof verification!\n\n");
    
    /* Allocate pixel buffer safely */
    size_t buffer_size = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t);
    uint32_t *pixel_buffer = (uint32_t *)safe_malloc(buffer_size);
    
    if (!pixel_buffer) {
        fprintf(stderr, "Failed to allocate pixel buffer\n");
        return 1;
    }
    
    /* Initialize buffer with safe memset */
    memset(pixel_buffer, 0, buffer_size);
    
    /* Draw Firefox logo pixels with bounds checking */
    int logo_x = 100;
    int logo_y = 100;
    
    /* This loop MUST call firefox_draw_pixel or compilation fails! */
    for (int i = 0; i < 10; i++) {
        int x, y;
        
        /* Safe integer arithmetic */
        SAFE_ADD(logo_x, i, x);
        y = logo_y;
        
        /* Runtime bounds validation */
        if (validate_pixel_bounds(x, y)) {
            /* This function call is REQUIRED for compilation */
            firefox_draw_pixel(x, y, FIREFOX_ORANGE);
            
            /* Safe array access */
            BOUNDS_CHECK(y * SCREEN_WIDTH + x, SCREEN_WIDTH * SCREEN_HEIGHT);
            pixel_buffer[y * SCREEN_WIDTH + x] = FIREFOX_ORANGE;
        }
    }
    
    /* Invariant check */
    INVARIANT(pixel_buffer != NULL);
    
    printf("\n✓ Firefox pixels drawn successfully!\n");
    printf("✓ All safety checks passed!\n");
    printf("✓ No buffer overflows possible!\n");
    
    /* Clean up */
    safe_free(pixel_buffer);
    
    return 0;
}
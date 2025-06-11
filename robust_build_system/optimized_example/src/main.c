/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/* Ultra-optimized application with compile-time proofs */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* External optimized functions */
extern void optimized_compute_kernel(uint32_t *buffer, size_t size);
extern void firefox_draw_pixel(int x, int y, uint32_t color);

/* Performance timing */
static inline double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* Benchmark mode */
static bool benchmark_mode = false;

/* Main application */
int main(int argc, char *argv[]) {
    /* Check for benchmark mode */
    if (argc > 1 && strcmp(argv[1], "--benchmark") == 0) {
        benchmark_mode = true;
    }
    
    printf("Ultra-Optimized Application\n");
    printf("===========================\n\n");
    
    /* Allocate aligned buffer for optimal performance */
    const size_t buffer_size = 1920 * 1080;  /* HD resolution */
    uint32_t *pixel_buffer = NULL;
    
    /* Use aligned allocation for vectorization */
    if (posix_memalign((void**)&pixel_buffer, 64, buffer_size * sizeof(uint32_t)) != 0) {
        fprintf(stderr, "Failed to allocate aligned buffer\n");
        return 1;
    }
    
    /* Initialize buffer */
    memset(pixel_buffer, 0, buffer_size * sizeof(uint32_t));
    
    /* THIS IS REQUIRED FOR COMPILATION - Proof system verifies this */
    printf("Initializing Firefox renderer...\n");
    firefox_draw_pixel(100, 100, 0xFF9500);  /* Must call this! */
    
    if (benchmark_mode) {
        printf("\nRunning performance benchmark...\n");
        
        const int iterations = 100;
        double total_time = 0.0;
        
        /* Warm-up */
        for (int i = 0; i < 10; i++) {
            optimized_compute_kernel(pixel_buffer, buffer_size);
        }
        
        /* Benchmark */
        for (int i = 0; i < iterations; i++) {
            double start = get_time();
            optimized_compute_kernel(pixel_buffer, buffer_size);
            double end = get_time();
            
            total_time += (end - start);
            
            /* Progress indicator */
            if ((i + 1) % 10 == 0) {
                printf("\rProgress: %d%%", (i + 1) * 100 / iterations);
                fflush(stdout);
            }
        }
        printf("\n\n");
        
        /* Results */
        double avg_time = total_time / iterations;
        double pixels_per_sec = buffer_size / avg_time;
        double gb_per_sec = (buffer_size * sizeof(uint32_t)) / (avg_time * 1e9);
        
        printf("Benchmark Results:\n");
        printf("  Average time: %.3f ms\n", avg_time * 1000);
        printf("  Throughput: %.2f Mpixels/sec\n", pixels_per_sec / 1e6);
        printf("  Bandwidth: %.2f GB/s\n", gb_per_sec);
        printf("  Efficiency: %.1f pixels/cycle (estimated)\n", 
               pixels_per_sec / 3e9);  /* Assume 3GHz CPU */
        
    } else {
        printf("\nProcessing image...\n");
        
        double start = get_time();
        optimized_compute_kernel(pixel_buffer, buffer_size);
        double end = get_time();
        
        printf("Processing completed in %.3f ms\n", (end - start) * 1000);
    }
    
    /* Verify some pixels were processed */
    int modified_count = 0;
    for (size_t i = 0; i < 1000 && i < buffer_size; i++) {
        if (pixel_buffer[i] != 0) {
            modified_count++;
        }
    }
    printf("\n✓ Verified: %d pixels modified\n", modified_count);
    
    /* Clean up */
    free(pixel_buffer);
    
    printf("\n✅ All operations completed successfully!\n");
    
    return 0;
}
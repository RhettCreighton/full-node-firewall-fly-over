/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/core/specification.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define MAX_SPECIFICATIONS 256
#define SIX_SIGMA_THRESHOLD 0.9999966  // 3.4 defects per million

static struct {
    specification_t* specs[MAX_SPECIFICATIONS];
    int count;
    bool initialized;
    FILE* log_file;
} g_spec_system = {0};

void spec_system_init(void) {
    if (g_spec_system.initialized) return;
    
    memset(&g_spec_system, 0, sizeof(g_spec_system));
    g_spec_system.initialized = true;
    
    // Open specification log
    char filename[256];
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    snprintf(filename, sizeof(filename), "spec_log_%04d%02d%02d_%02d%02d%02d.txt",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
    
    g_spec_system.log_file = fopen(filename, "w");
    if (g_spec_system.log_file) {
        fprintf(g_spec_system.log_file, "=== SPECIFICATION DRIVEN DEVELOPMENT LOG ===\n");
        fprintf(g_spec_system.log_file, "Six Sigma Target: %.6f%% reliability\n\n", 
                SIX_SIGMA_THRESHOLD * 100);
    }
}

void spec_system_shutdown(void) {
    if (!g_spec_system.initialized) return;
    
    // Generate final report
    spec_report_all();
    
    if (g_spec_system.log_file) {
        fclose(g_spec_system.log_file);
    }
    
    g_spec_system.initialized = false;
}

specification_t* spec_create(const char* name, const char* description) {
    specification_t* spec = calloc(1, sizeof(specification_t));
    if (!spec) {
        fprintf(stderr, "FATAL: Cannot allocate specification\n");
        abort();
    }
    
    spec->name = name;
    spec->description = description;
    spec->status = SPEC_STATUS_PENDING;
    spec->reliability_percentage = 100.0;
    
    return spec;
}

void spec_register(specification_t* spec) {
    // GRACEFUL: Auto-initialize if needed instead of crashing
    if (!g_spec_system.initialized) {
        fprintf(stderr, "WARNING: Spec system not initialized, auto-initializing...\n");
        spec_system_init();
    }
    
    if (!spec) {
        fprintf(stderr, "ERROR: Cannot register NULL specification\n");
        return;
    }
    
    if (g_spec_system.count >= MAX_SPECIFICATIONS) {
        fprintf(stderr, "ERROR: Too many specifications (max %d)\n", MAX_SPECIFICATIONS);
        spec->status = SPEC_STATUS_FAILED;
        return;
    }
    
    g_spec_system.specs[g_spec_system.count++] = spec;
    spec->status = SPEC_STATUS_RUNNING;
    
    if (g_spec_system.log_file) {
        fprintf(g_spec_system.log_file, "[SPEC] Registered: %s\n", spec->name);
        fprintf(g_spec_system.log_file, "      Description: %s\n\n", spec->description);
    }
}

bool spec_check_condition(specification_t* spec, bool condition,
                         const char* expr, const char* msg,
                         const char* file, int line) {
    spec->checks_total++;
    
    if (condition) {
        spec->checks_passed++;
    } else {
        spec->checks_failed++;
        snprintf(spec->failure_reason, sizeof(spec->failure_reason),
                 "Check failed: %s\nMessage: %s\nLocation: %s:%d",
                 expr, msg, file, line);
        
        // Log failure
        if (g_spec_system.log_file) {
            fprintf(g_spec_system.log_file, 
                    "[FAIL] %s: %s at %s:%d\n", spec->name, msg, file, line);
        }
        
        // Critical specs log error but don't crash
        if (spec->status == SPEC_STATUS_CRITICAL_FAILURE) {
            fprintf(stderr, "\n=== CRITICAL SPECIFICATION FAILURE ===\n");
            fprintf(stderr, "Spec: %s\n", spec->name);
            fprintf(stderr, "%s\n", spec->failure_reason);
            fprintf(stderr, "=====================================\n");
            // DON'T CRASH! Just mark as failed
            spec->status = SPEC_STATUS_FAILED;
        }
    }
    
    // Update reliability
    spec->reliability_percentage = 
        (double)spec->checks_passed / spec->checks_total * 100.0;
    
    return condition;
}

bool spec_check_not_null(specification_t* spec, const void* ptr,
                        const char* ptr_name, const char* msg,
                        const char* file, int line) {
    char full_msg[512];
    snprintf(full_msg, sizeof(full_msg), "%s (pointer '%s' is NULL)", msg, ptr_name);
    return spec_check_condition(spec, ptr != NULL, ptr_name, full_msg, file, line);
}

bool spec_check_range(specification_t* spec, double value,
                     double min, double max, const char* value_name,
                     const char* msg, const char* file, int line) {
    char full_msg[512];
    snprintf(full_msg, sizeof(full_msg), 
             "%s (%s=%.2f not in range [%.2f, %.2f])", 
             msg, value_name, value, min, max);
    
    bool in_range = (value >= min && value <= max);
    return spec_check_condition(spec, in_range, value_name, full_msg, file, line);
}

bool spec_check_pixels(specification_t* spec, const void* buffer,
                      int width, int height, const char* msg,
                      const char* file, int line) {
    if (!buffer) {
        return spec_check_not_null(spec, buffer, "pixel_buffer", msg, file, line);
    }
    
    // Check if buffer contains any non-zero pixels
    const unsigned char* pixels = (const unsigned char*)buffer;
    int pixel_count = width * height * 4; // Assuming RGBA
    bool has_pixels = false;
    
    for (int i = 0; i < pixel_count; i++) {
        if (pixels[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    
    char full_msg[512];
    snprintf(full_msg, sizeof(full_msg), 
             "%s (no visible pixels in %dx%d buffer)", msg, width, height);
    
    return spec_check_condition(spec, has_pixels, "pixel_content", full_msg, file, line);
}

bool spec_check_memory(specification_t* spec, const void* ptr,
                      size_t size, const char* msg,
                      const char* file, int line) {
    if (!spec_check_not_null(spec, ptr, "memory", msg, file, line)) {
        return false;
    }
    
    // Try to read from the memory to verify it's accessible
    volatile unsigned char test = 0;
    for (size_t i = 0; i < size; i += 4096) { // Check every page
        test += ((const unsigned char*)ptr)[i];
    }
    if (size > 0) {
        test += ((const unsigned char*)ptr)[size - 1]; // Check last byte
    }
    
    return true;
}

bool spec_finalize(specification_t* spec) {
    if (spec->checks_failed > 0) {
        spec->status = SPEC_STATUS_FAILED;
    } else if (spec->checks_total > 0) {
        spec->status = SPEC_STATUS_PASSED;
    }
    
    // Check Six Sigma threshold
    double success_rate = spec->reliability_percentage / 100.0;
    bool meets_six_sigma = success_rate >= SIX_SIGMA_THRESHOLD;
    
    if (!meets_six_sigma && spec->status == SPEC_STATUS_PASSED) {
        spec->status = SPEC_STATUS_FAILED;
        snprintf(spec->failure_reason, sizeof(spec->failure_reason),
                 "Below Six Sigma threshold: %.6f%% < %.6f%%",
                 spec->reliability_percentage, SIX_SIGMA_THRESHOLD * 100);
    }
    
    return spec->status == SPEC_STATUS_PASSED;
}

void spec_report(specification_t* spec) {
    printf("\n=== SPECIFICATION REPORT: %s ===\n", spec->name);
    printf("Description: %s\n", spec->description);
    printf("Status: %s\n", 
           spec->status == SPEC_STATUS_PASSED ? "PASSED" :
           spec->status == SPEC_STATUS_FAILED ? "FAILED" :
           spec->status == SPEC_STATUS_CRITICAL_FAILURE ? "CRITICAL FAILURE" :
           "PENDING");
    printf("Checks: %d passed, %d failed (total: %d)\n",
           spec->checks_passed, spec->checks_failed, spec->checks_total);
    printf("Reliability: %.6f%%\n", spec->reliability_percentage);
    
    if (spec->status == SPEC_STATUS_FAILED || 
        spec->status == SPEC_STATUS_CRITICAL_FAILURE) {
        printf("Failure reason: %s\n", spec->failure_reason);
    }
    
    printf("Six Sigma: %s\n", 
           spec->reliability_percentage >= SIX_SIGMA_THRESHOLD * 100 ? "MET" : "NOT MET");
    printf("=====================================\n");
}

void spec_report_all(void) {
    printf("\n=== ALL SPECIFICATIONS SUMMARY ===\n");
    int passed = 0, failed = 0, pending = 0;
    
    for (int i = 0; i < g_spec_system.count; i++) {
        specification_t* spec = g_spec_system.specs[i];
        switch (spec->status) {
            case SPEC_STATUS_PASSED: passed++; break;
            case SPEC_STATUS_FAILED:
            case SPEC_STATUS_CRITICAL_FAILURE: failed++; break;
            default: pending++; break;
        }
        
        printf("%2d. %-30s: %s (%.2f%%)\n", 
               i + 1, spec->name,
               spec->status == SPEC_STATUS_PASSED ? "PASS" :
               spec->status == SPEC_STATUS_FAILED ? "FAIL" : "PEND",
               spec->reliability_percentage);
    }
    
    printf("\nTotal: %d passed, %d failed, %d pending\n", passed, failed, pending);
    printf("==================================\n");
}

void spec_set_critical(specification_t* spec, bool critical) {
    if (critical && spec->status == SPEC_STATUS_FAILED) {
        spec->status = SPEC_STATUS_CRITICAL_FAILURE;
    }
}
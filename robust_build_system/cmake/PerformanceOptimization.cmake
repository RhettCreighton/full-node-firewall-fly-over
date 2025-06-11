# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# PerformanceOptimization.cmake - Advanced performance optimization

include_guard(GLOBAL)

# CPU feature detection
include(CheckCSourceCompiles)

# Detect CPU features at configure time
function(detect_cpu_features)
    # Check for AVX2
    check_c_source_compiles("
        #include <immintrin.h>
        int main() {
            __m256i a = _mm256_set1_epi32(1);
            return 0;
        }
    " HAS_AVX2)
    
    # Check for AVX-512
    check_c_source_compiles("
        #include <immintrin.h>
        int main() {
            __m512i a = _mm512_set1_epi32(1);
            return 0;
        }
    " HAS_AVX512)
    
    # Check for ARM NEON
    check_c_source_compiles("
        #include <arm_neon.h>
        int main() {
            int32x4_t a = vdupq_n_s32(1);
            return 0;
        }
    " HAS_NEON)
    
    # Set appropriate flags
    if(HAS_AVX512)
        set(CPU_OPTIMIZATION_FLAGS "-march=native -mavx512f" PARENT_SCOPE)
        message(STATUS "✓ AVX-512 support detected")
    elseif(HAS_AVX2)
        set(CPU_OPTIMIZATION_FLAGS "-march=native -mavx2" PARENT_SCOPE)
        message(STATUS "✓ AVX2 support detected")
    elseif(HAS_NEON)
        set(CPU_OPTIMIZATION_FLAGS "-march=native -mfpu=neon" PARENT_SCOPE)
        message(STATUS "✓ ARM NEON support detected")
    else()
        set(CPU_OPTIMIZATION_FLAGS "-march=native" PARENT_SCOPE)
    endif()
endfunction()

# Automatic vectorization analysis
function(enable_vectorization_report TARGET)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        target_compile_options(${TARGET} PRIVATE
            -ftree-vectorize
            -fopt-info-vec-optimized
            -fopt-info-vec-missed
        )
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${TARGET} PRIVATE
            -Rpass=loop-vectorize
            -Rpass-missed=loop-vectorize
            -Rpass-analysis=loop-vectorize
        )
    elseif(CMAKE_C_COMPILER_ID MATCHES "Intel")
        target_compile_options(${TARGET} PRIVATE
            -qopt-report=5
            -qopt-report-phase=vec
        )
    endif()
endfunction()

# Memory access optimization
function(optimize_memory_access TARGET)
    # Prefetching
    target_compile_options(${TARGET} PRIVATE
        -fprefetch-loop-arrays
    )
    
    # Alignment
    target_compile_definitions(${TARGET} PRIVATE
        _MALLOC_ALIGNMENT=64  # For AVX-512
    )
    
    # Cache optimization
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${TARGET} PRIVATE
            -fdata-sections
            -ffunction-sections
            --param l1-cache-size=32
            --param l1-cache-line-size=64
            --param l2-cache-size=256
        )
    endif()
endfunction()

# Branch prediction optimization
function(optimize_branch_prediction TARGET)
    # Profile-guided branch optimization
    target_compile_options(${TARGET} PRIVATE
        -fbranch-probabilities
        -freorder-blocks-and-partition
    )
    
    # Generate branch prediction hints
    target_compile_definitions(${TARGET} PRIVATE
        "likely(x)=__builtin_expect(!!(x),1)"
        "unlikely(x)=__builtin_expect(!!(x),0)"
    )
endfunction()

# Hardware performance counter integration
function(add_performance_counters TARGET)
    set(PERF_HEADER "${CMAKE_BINARY_DIR}/${TARGET}_perf.h")
    
    file(WRITE ${PERF_HEADER} "
#ifndef ${TARGET}_PERF_H
#define ${TARGET}_PERF_H

#ifdef __linux__
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                           int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

typedef struct {
    int fd;
    long long count;
} perf_counter_t;

static inline perf_counter_t perf_start_counter(int type, int config) {
    perf_counter_t counter = {-1, 0};
    
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(pe));
    pe.type = type;
    pe.size = sizeof(pe);
    pe.config = config;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    
    counter.fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (counter.fd >= 0) {
        ioctl(counter.fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(counter.fd, PERF_EVENT_IOC_ENABLE, 0);
    }
    
    return counter;
}

static inline long long perf_read_counter(perf_counter_t *counter) {
    if (counter->fd < 0) return -1;
    
    ioctl(counter->fd, PERF_EVENT_IOC_DISABLE, 0);
    read(counter->fd, &counter->count, sizeof(counter->count));
    close(counter->fd);
    
    return counter->count;
}

#define PERF_COUNT_CYCLES() perf_start_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES)
#define PERF_COUNT_INSTRUCTIONS() perf_start_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS)
#define PERF_COUNT_CACHE_MISSES() perf_start_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES)
#define PERF_COUNT_BRANCH_MISSES() perf_start_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES)

#else
// Fallback for non-Linux
typedef struct { int dummy; } perf_counter_t;
#define PERF_COUNT_CYCLES() ((perf_counter_t){0})
#define PERF_COUNT_INSTRUCTIONS() ((perf_counter_t){0})
#define PERF_COUNT_CACHE_MISSES() ((perf_counter_t){0})
#define PERF_COUNT_BRANCH_MISSES() ((perf_counter_t){0})
static inline long long perf_read_counter(perf_counter_t *c) { return -1; }
#endif

#endif /* ${TARGET}_PERF_H */
")
    
    target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR})
endfunction()

# Compilation database optimization
function(optimize_compilation_database)
    # Use compiler cache
    find_program(CCACHE_PROGRAM ccache)
    find_program(SCCACHE_PROGRAM sccache)
    
    if(SCCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM} PARENT_SCOPE)
        message(STATUS "✓ Using sccache for compilation")
    elseif(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM} PARENT_SCOPE)
        message(STATUS "✓ Using ccache for compilation")
    endif()
    
    # Enable color diagnostics
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endfunction()

# Parallel compilation optimization
function(optimize_parallel_compilation)
    # Use ninja if available
    find_program(NINJA_PROGRAM ninja)
    if(NINJA_PROGRAM AND NOT CMAKE_GENERATOR MATCHES "Ninja")
        message(STATUS "💡 Consider using Ninja generator for faster builds:")
        message(STATUS "   cmake -G Ninja ..")
    endif()
    
    # Set parallel link jobs
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
        cmake_host_system_information(RESULT TOTAL_MEMORY QUERY TOTAL_PHYSICAL_MEMORY)
        
        # Limit parallel link jobs based on available memory
        # Each link job can use ~2GB of memory
        math(EXPR LINK_JOBS "${TOTAL_MEMORY} / 2048")
        if(LINK_JOBS LESS 1)
            set(LINK_JOBS 1)
        elseif(LINK_JOBS GREATER 4)
            set(LINK_JOBS 4)  # Cap at 4 to be safe
        endif()
        
        set_property(GLOBAL PROPERTY JOB_POOLS link_job_pool=${LINK_JOBS})
        set(CMAKE_JOB_POOL_LINK link_job_pool PARENT_SCOPE)
        
        message(STATUS "✓ Parallel linking with ${LINK_JOBS} jobs")
    endif()
endfunction()

# Whole program optimization
function(enable_whole_program_optimization TARGET)
    # Interprocedural optimization
    check_ipo_supported(RESULT IPO_SUPPORTED)
    if(IPO_SUPPORTED)
        set_property(TARGET ${TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    
    # Whole program visibility
    target_compile_options(${TARGET} PRIVATE
        -fwhole-program
        -fvisibility=hidden
    )
    
    # Link-time optimization flags
    target_link_options(${TARGET} PRIVATE
        -Wl,--gc-sections
        -Wl,--icf=all
    )
endfunction()

# Auto-tuning support
function(enable_auto_tuning TARGET)
    set(TUNING_SOURCE "${CMAKE_BINARY_DIR}/${TARGET}_tune.c")
    
    file(WRITE ${TUNING_SOURCE} "
#include <stdio.h>
#include <time.h>
#include <string.h>

// Tuning parameters
static struct {
    int unroll_factor;
    int block_size;
    int prefetch_distance;
} tuning_params = {4, 64, 8};

void tune_parameters() {
    // Simple auto-tuning of block size
    int best_block = 64;
    double best_time = 1e9;
    
    for (int block = 16; block <= 256; block *= 2) {
        clock_t start = clock();
        
        // Benchmark with this block size
        // ... your benchmark code ...
        
        clock_t end = clock();
        double time = (double)(end - start) / CLOCKS_PER_SEC;
        
        if (time < best_time) {
            best_time = time;
            best_block = block;
        }
    }
    
    tuning_params.block_size = best_block;
    
    // Save tuning results
    FILE *f = fopen(\"${CMAKE_BINARY_DIR}/${TARGET}_tuning.h\", \"w\");
    if (f) {
        fprintf(f, \"#define TUNED_BLOCK_SIZE %d\\n\", best_block);
        fprintf(f, \"#define TUNED_UNROLL %d\\n\", tuning_params.unroll_factor);
        fprintf(f, \"#define TUNED_PREFETCH %d\\n\", tuning_params.prefetch_distance);
        fclose(f);
    }
}

// Run tuning at startup
__attribute__((constructor))
void auto_tune_init() {
    static int tuned = 0;
    if (!tuned) {
        tuned = 1;
        tune_parameters();
    }
}
")
    
    target_sources(${TARGET} PRIVATE ${TUNING_SOURCE})
endfunction()

# Summary function
function(print_performance_optimization_summary)
    message(STATUS "")
    message(STATUS "Performance Optimizations:")
    
    detect_cpu_features()
    if(CPU_OPTIMIZATION_FLAGS)
        message(STATUS "  CPU flags: ${CPU_OPTIMIZATION_FLAGS}")
    endif()
    
    get_property(LAUNCHER GLOBAL PROPERTY RULE_LAUNCH_COMPILE)
    if(LAUNCHER)
        message(STATUS "  ✓ Compiler cache: ${LAUNCHER}")
    endif()
    
    if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        message(STATUS "  ✓ Link-time optimization")
    endif()
    
    ProcessorCount(NPROC)
    message(STATUS "  ✓ Parallel compilation: ${NPROC} cores")
    
    message(STATUS "")
endfunction()
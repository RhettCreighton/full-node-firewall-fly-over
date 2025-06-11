# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)
project(UltraOptimizedApp VERSION 1.0.0 LANGUAGES C CXX)

# Use our advanced build system
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

# Include all optimization modules
include(AdvancedBuildOptimization)
include(OptimizedCompileTimeProof)
include(C99Compliance)
include(SafetyChecks)
include(SingleBinary)

# Enable all optimizations from the start
apply_ultimate_build_optimization()

# Options
option(ENABLE_ULTRA_PROOF "Use ultra-fast proof system" ON)
option(ENABLE_DISTRIBUTED "Enable distributed compilation" ON)
option(ENABLE_PROFILING "Enable build profiling" ON)

# Source files
set(SOURCES
    src/main.c
    src/compute_kernel.c
    src/firefox_renderer.c
)

# Create single optimized binary
add_executable(UltraOptimizedApp ${SOURCES})

# Apply C99 compliance
apply_c99_compliance(UltraOptimizedApp)

# Apply safety checks
apply_safety_checks(UltraOptimizedApp)

# Enable ultimate optimization
enable_ultimate_optimization(UltraOptimizedApp)

# Ultra-fast proof verification
if(ENABLE_ULTRA_PROOF)
    add_ultra_fast_proof(UltraOptimizedApp
        FUNCTION firefox_draw_pixel
        SOURCES ${SOURCES}
    )
else()
    # Fallback to optimized proof with caching
    add_optimized_compile_time_proof(UltraOptimizedApp
        PROOF_FUNCTION firefox_draw_pixel
        EXPECTED_OUTPUT "Drawing pixel"
        SOURCES ${SOURCES}
        METHOD static  # Fastest method
    )
endif()

# LLVM pass verification if available
if(LLVM_FOUND)
    add_llvm_proof_verification(UltraOptimizedApp firefox_draw_pixel)
endif()

# Link with system PCH for faster builds
if(TARGET system_pch)
    target_link_libraries(UltraOptimizedApp PRIVATE system_pch)
endif()

# Enable unity build for this target
set_target_properties(UltraOptimizedApp PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE 8
)

# Custom optimization flags for compute kernel
set_source_files_properties(src/compute_kernel.c PROPERTIES
    COMPILE_FLAGS "-march=native -funroll-all-loops -fprefetch-loop-arrays"
)

# Installation
install(TARGETS UltraOptimizedApp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Performance benchmark target
add_custom_target(benchmark
    COMMAND ${CMAKE_COMMAND} -E echo "Running performance benchmark..."
    COMMAND perf stat -e cycles,instructions,cache-misses,branch-misses 
            $<TARGET_FILE:UltraOptimizedApp> --benchmark
    DEPENDS UltraOptimizedApp
    COMMENT "Benchmarking with hardware performance counters..."
)

# Build statistics
add_custom_target(build_stats
    COMMAND ${CMAKE_COMMAND} -E echo "Build Statistics:"
    COMMAND wc -l ${CMAKE_BINARY_DIR}/build_times.csv 2>/dev/null || echo "No profiling data"
    COMMAND du -sh ${CMAKE_BINARY_DIR} | cut -f1 | xargs echo "Build directory size:"
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.o" | wc -l | xargs echo "Object files:"
    COMMAND ${CMAKE_COMMAND} -E echo "Cache statistics:"
    COMMAND ccache -s 2>/dev/null || echo "No ccache stats"
    COMMENT "Gathering build statistics..."
)

message(STATUS "")
message(STATUS "=== Ultra-Optimized Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "")
message(STATUS "Quick commands:")
message(STATUS "  Build:      ninja -j$(nproc)")
message(STATUS "  Benchmark:  ninja benchmark")
message(STATUS "  Stats:      ninja build_stats")
message(STATUS "===========================================")
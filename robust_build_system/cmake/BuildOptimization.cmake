# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# BuildOptimization.cmake - Advanced build optimization and caching

include_guard(GLOBAL)

# Enable ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "✓ Using ccache for faster builds")
endif()

# Profile-guided optimization support
option(ENABLE_PGO_GENERATE "Generate profile data" OFF)
option(ENABLE_PGO_USE "Use profile data for optimization" OFF)

# Configure optimization flags based on build type
function(configure_optimization_flags)
    # Base optimization flags for different levels
    set(OPT_FLAGS_DEBUG "-O0 -g3 -fno-omit-frame-pointer")
    set(OPT_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    set(OPT_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(OPT_FLAGS_MINSIZEREL "-Os -DNDEBUG")
    
    # Advanced optimizations for Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # Loop optimizations
        set(LOOP_OPTS
            -ftree-loop-vectorize
            -ftree-slp-vectorize
            -floop-interchange
            -floop-strip-mine
            -floop-block
        )
        
        # Function optimizations
        set(FUNC_OPTS
            -finline-functions
            -finline-limit=1000
            -fwhole-program
        )
        
        # Memory optimizations
        set(MEM_OPTS
            -foptimize-strlen
            -fmerge-all-constants
            -fgcse-after-reload
        )
        
        # Math optimizations (careful with these)
        if(NOT STRICT_MATH)
            set(MATH_OPTS
                -ffast-math
                -fno-math-errno
                -ffinite-math-only
            )
        endif()
    endif()
    
    # Profile-guided optimization
    if(ENABLE_PGO_GENERATE)
        add_compile_options(-fprofile-generate)
        add_link_options(-fprofile-generate)
        message(STATUS "✓ PGO generation enabled")
    elseif(ENABLE_PGO_USE)
        add_compile_options(-fprofile-use)
        add_link_options(-fprofile-use)
        message(STATUS "✓ PGO optimization enabled")
    endif()
endfunction()

# Unity build support for faster compilation
function(enable_unity_build TARGET)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
        set_target_properties(${TARGET} PROPERTIES UNITY_BUILD ON)
        set_target_properties(${TARGET} PROPERTIES UNITY_BUILD_BATCH_SIZE 16)
        message(STATUS "✓ Unity build enabled for ${TARGET}")
    endif()
endfunction()

# Precompiled header support
function(add_precompiled_header TARGET HEADER)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
        target_precompile_headers(${TARGET} PRIVATE ${HEADER})
        message(STATUS "✓ Precompiled header enabled for ${TARGET}")
    endif()
endfunction()

# Link-time optimization configuration
function(configure_lto TARGET)
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        include(CheckIPOSupported)
        check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT ERROR)
        
        if(LTO_SUPPORTED)
            set_property(TARGET ${TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
            message(STATUS "✓ Link-time optimization enabled for ${TARGET}")
        endif()
    endif()
endfunction()

# Binary size optimization
function(optimize_binary_size TARGET)
    if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR MINIMIZE_SIZE)
        # Strip unnecessary sections
        target_link_options(${TARGET} PRIVATE
            -Wl,--gc-sections
            -Wl,--strip-all
            -Wl,--as-needed
        )
        
        # Optimize for size
        target_compile_options(${TARGET} PRIVATE
            -Os
            -ffunction-sections
            -fdata-sections
            -fno-unwind-tables
            -fno-asynchronous-unwind-tables
        )
        
        # Consider using musl for smaller binaries
        if(USE_MUSL)
            target_link_options(${TARGET} PRIVATE -static)
        endif()
    endif()
endfunction()

# Parallel build configuration
function(configure_parallel_build)
    # Determine number of cores
    include(ProcessorCount)
    ProcessorCount(NPROC)
    
    if(NOT NPROC EQUAL 0)
        # Set parallel jobs for make
        if(CMAKE_GENERATOR MATCHES "Make")
            set(CMAKE_BUILD_PARALLEL_LEVEL ${NPROC} PARENT_SCOPE)
        endif()
        
        # Configure ninja parallel jobs
        if(CMAKE_GENERATOR MATCHES "Ninja")
            set(CMAKE_JOB_POOL_COMPILE "compile_job_pool")
            set_property(GLOBAL PROPERTY JOB_POOLS compile_job_pool=${NPROC})
        endif()
        
        message(STATUS "✓ Parallel build with ${NPROC} jobs")
    endif()
endfunction()

# Build time analysis
function(enable_build_timing)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE 
            "${CMAKE_COMMAND} -E time")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK 
            "${CMAKE_COMMAND} -E time")
    endif()
endfunction()

# Optimization summary
function(print_optimization_summary)
    message(STATUS "")
    message(STATUS "Build Optimization Summary:")
    message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
    
    if(CCACHE_FOUND)
        message(STATUS "  ✓ ccache enabled")
    endif()
    
    if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        message(STATUS "  ✓ LTO enabled")
    endif()
    
    if(ENABLE_NATIVE_ARCH)
        message(STATUS "  ✓ Native architecture optimization")
    endif()
    
    get_property(PARALLEL_LEVEL GLOBAL PROPERTY CMAKE_BUILD_PARALLEL_LEVEL)
    if(PARALLEL_LEVEL)
        message(STATUS "  ✓ Parallel build: ${PARALLEL_LEVEL} jobs")
    endif()
    
    message(STATUS "")
endfunction()
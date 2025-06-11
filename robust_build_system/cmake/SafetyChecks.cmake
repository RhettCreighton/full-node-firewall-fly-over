# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# SafetyChecks.cmake - Runtime and compile-time safety verification

include_guard(GLOBAL)

# Apply comprehensive safety checks to a target
function(apply_safety_checks TARGET)
    # Stack protection
    target_compile_options(${TARGET} PRIVATE
        -fstack-protector-strong
        -fstack-clash-protection
    )
    
    # Fortify source
    target_compile_definitions(${TARGET} PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>
    )
    
    # Position Independent Executable
    set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_options(${TARGET} PRIVATE -pie)
    
    # Additional hardening flags
    target_compile_options(${TARGET} PRIVATE
        -Wformat
        -Wformat-security
        -Werror=format-security
        -D_GLIBCXX_ASSERTIONS
    )
    
    # RELRO (Relocation Read-Only)
    target_link_options(${TARGET} PRIVATE
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-z,noexecstack
    )
    
    # Control flow integrity (if supported)
    include(CheckCCompilerFlag)
    check_c_compiler_flag("-fcf-protection" HAS_CF_PROTECTION)
    if(HAS_CF_PROTECTION)
        target_compile_options(${TARGET} PRIVATE -fcf-protection)
    endif()
endfunction()

# Add runtime assertions
function(add_runtime_assertions TARGET)
    set(ASSERT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_assert.h")
    
    file(WRITE ${ASSERT_HEADER} "
/* Runtime assertion system */
#ifndef ${TARGET}_ASSERT_H
#define ${TARGET}_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define RUNTIME_ASSERT(cond, msg) do { \\
    if (!(cond)) { \\
        fprintf(stderr, \"Assertion failed at %s:%d: %s\\n\", \\
                __FILE__, __LINE__, msg); \\
        abort(); \\
    } \\
} while(0)

#define BOUNDS_CHECK(idx, size) \\
    RUNTIME_ASSERT((idx) >= 0 && (idx) < (size), \"Bounds check failed\")

#define NULL_CHECK(ptr) \\
    RUNTIME_ASSERT((ptr) != NULL, \"Null pointer check failed\")

#define INVARIANT(expr) \\
    RUNTIME_ASSERT(expr, \"Invariant violation: \" #expr)

#endif /* ${TARGET}_ASSERT_H */
")
    
    target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_definitions(${TARGET} PRIVATE USE_RUNTIME_ASSERTIONS)
endfunction()

# Memory safety checks
function(add_memory_safety_checks TARGET)
    # AddressSanitizer for Debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
        target_compile_options(${TARGET} PRIVATE -fsanitize=address)
        target_link_options(${TARGET} PRIVATE -fsanitize=address)
        message(STATUS "✓ AddressSanitizer enabled for ${TARGET}")
    endif()
    
    # Create custom memory functions
    set(MEM_SAFETY_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_memsafety.c")
    
    file(WRITE ${MEM_SAFETY_SOURCE} "
/* Memory safety wrappers */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void *safe_malloc(size_t size) {
    if (size == 0) return NULL;
    void *ptr = malloc(size);
    assert(ptr != NULL && \"Memory allocation failed\");
    return ptr;
}

void *safe_calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) return NULL;
    void *ptr = calloc(nmemb, size);
    assert(ptr != NULL && \"Memory allocation failed\");
    return ptr;
}

void safe_free(void *ptr) {
    free(ptr);
    /* Note: Can't set to NULL here as ptr is passed by value */
}

char *safe_strncpy(char *dest, const char *src, size_t n) {
    if (n == 0) return dest;
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\\0';
    return dest;
}
")
    
    target_sources(${TARGET} PRIVATE ${MEM_SAFETY_SOURCE})
endfunction()

# Integer overflow checks
function(add_integer_overflow_checks TARGET)
    include(CheckCCompilerFlag)
    
    # Check for compiler support
    check_c_compiler_flag("-fsanitize=integer" HAS_INT_SANITIZER)
    check_c_compiler_flag("-ftrapv" HAS_TRAPV)
    
    if(HAS_INT_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET} PRIVATE -fsanitize=integer)
        target_link_options(${TARGET} PRIVATE -fsanitize=integer)
    elseif(HAS_TRAPV)
        target_compile_options(${TARGET} PRIVATE -ftrapv)
    endif()
    
    # Add safe integer operations
    set(INT_SAFETY_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_intsafe.h")
    
    file(WRITE ${INT_SAFETY_HEADER} "
/* Safe integer operations */
#ifndef ${TARGET}_INTSAFE_H
#define ${TARGET}_INTSAFE_H

#include <limits.h>
#include <stdbool.h>

static inline bool safe_add(int a, int b, int *result) {
    if (b > 0 && a > INT_MAX - b) return false;
    if (b < 0 && a < INT_MIN - b) return false;
    *result = a + b;
    return true;
}

static inline bool safe_multiply(int a, int b, int *result) {
    if (a > 0 && b > 0 && a > INT_MAX / b) return false;
    if (a > 0 && b < 0 && b < INT_MIN / a) return false;
    if (a < 0 && b > 0 && a < INT_MIN / b) return false;
    if (a < 0 && b < 0 && a < INT_MAX / b) return false;
    *result = a * b;
    return true;
}

#define SAFE_ADD(a, b, result) \\
    if (!safe_add(a, b, &(result))) { \\
        fprintf(stderr, \"Integer overflow in addition\\n\"); \\
        abort(); \\
    }

#define SAFE_MULTIPLY(a, b, result) \\
    if (!safe_multiply(a, b, &(result))) { \\
        fprintf(stderr, \"Integer overflow in multiplication\\n\"); \\
        abort(); \\
    }

#endif /* ${TARGET}_INTSAFE_H */
")
    
    target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()

# Static analysis integration
function(add_static_analysis TARGET)
    # Clang-tidy
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CLANG_TIDY_CHECKS
            "-checks="
            "-*,"
            "bugprone-*,"
            "cert-*,"
            "clang-analyzer-*,"
            "misc-*,"
            "performance-*,"
            "portability-*,"
            "readability-*"
        )
        string(REPLACE ";" "" CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS}")
        
        set_target_properties(${TARGET} PROPERTIES
            C_CLANG_TIDY "${CLANG_TIDY_EXE};${CLANG_TIDY_CHECKS}"
        )
        message(STATUS "✓ clang-tidy enabled for ${TARGET}")
    endif()
    
    # cppcheck
    find_program(CPPCHECK_EXE NAMES "cppcheck")
    if(CPPCHECK_EXE)
        set(CPPCHECK_ARGS
            "--enable=all"
            "--suppress=missingIncludeSystem"
            "--inline-suppr"
            "--quiet"
        )
        
        set_target_properties(${TARGET} PROPERTIES
            C_CPPCHECK "${CPPCHECK_EXE};${CPPCHECK_ARGS}"
        )
        message(STATUS "✓ cppcheck enabled for ${TARGET}")
    endif()
endfunction()

# Fuzzing support
function(add_fuzzing_support TARGET)
    if(ENABLE_FUZZING)
        # Check for fuzzer support
        include(CheckCCompilerFlag)
        check_c_compiler_flag("-fsanitize=fuzzer" HAS_FUZZER)
        
        if(HAS_FUZZER)
            # Create a fuzzing variant of the target
            set(FUZZ_TARGET "${TARGET}_fuzz")
            add_executable(${FUZZ_TARGET} EXCLUDE_FROM_ALL ${ARGN})
            
            target_compile_options(${FUZZ_TARGET} PRIVATE
                -fsanitize=fuzzer,address
                -g
            )
            
            target_link_options(${FUZZ_TARGET} PRIVATE
                -fsanitize=fuzzer,address
            )
            
            message(STATUS "✓ Fuzzing target created: ${FUZZ_TARGET}")
        endif()
    endif()
endfunction()
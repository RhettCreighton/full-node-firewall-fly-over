# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# C99Compliance.cmake - Enforce strict C99 compliance

include_guard(GLOBAL)

# Check if compiler supports C99
include(CheckCCompilerFlag)

# C99 compliance flags
set(C99_FLAGS
    -std=c99
    -pedantic
    -Wdeclaration-after-statement
    -Wold-style-definition
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wc++-compat
)

# Additional strictness flags
set(STRICT_FLAGS
    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wfloat-equal
    -Wformat=2
    -Wformat-security
    -Wmissing-include-dirs
    -Wpointer-arith
    -Wredundant-decls
    -Wshadow
    -Wswitch-default
    -Wswitch-enum
    -Wundef
    -Wuninitialized
    -Wunreachable-code
    -Wunused
    -Wwrite-strings
)

# Apply C99 compliance to a target
function(apply_c99_compliance TARGET)
    # Set C99 standard
    set_target_properties(${TARGET} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
    
    # Add C99 flags
    foreach(FLAG ${C99_FLAGS})
        target_compile_options(${TARGET} PRIVATE ${FLAG})
    endforeach()
    
    # Add strict flags in Debug mode
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        foreach(FLAG ${STRICT_FLAGS})
            # Check if compiler supports the flag
            string(REPLACE "-" "_" FLAG_VAR "HAS_FLAG${FLAG}")
            check_c_compiler_flag(${FLAG} ${FLAG_VAR})
            if(${FLAG_VAR})
                target_compile_options(${TARGET} PRIVATE ${FLAG})
            endif()
        endforeach()
    endif()
    
    # Add definitions for C99 features
    target_compile_definitions(${TARGET} PRIVATE
        _ISOC99_SOURCE
        _POSIX_C_SOURCE=200809L
    )
endfunction()

# Verify source files are C99 compliant
function(verify_c99_sources)
    set(C99_TEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/c99_test.c")
    
    # Create test file with C99 features
    file(WRITE ${C99_TEST_FILE} "
/* C99 compliance test */
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

/* C99 inline function */
static inline int square(int x) { return x * x; }

/* C99 designated initializers */
struct point { int x, y; };
static struct point p = { .y = 2, .x = 1 };

/* C99 compound literals */
static int *ptr = (int[]){1, 2, 3};

/* C99 variable-length arrays (VLA) - we actually forbid these */
/* int vla[n]; -- This should cause an error with -Wvla */

/* C99 // comments */
// This is a C99 comment

/* C99 mixed declarations and code */
int main(void) {
    int a = 1;
    printf(\"Testing C99\\n\");
    int b = 2;  /* C99 allows this */
    
    /* C99 for loop declarations */
    for (int i = 0; i < 10; i++) {
        /* C99 bool type */
        bool flag = true;
        (void)flag;
    }
    
    /* C99 variadic macros */
    #define DEBUG(...) printf(__VA_ARGS__)
    DEBUG(\"Values: %d %d\\n\", a, b);
    
    return 0;
}
")
    
    # Try to compile with strict C99
    try_compile(C99_SUPPORTED
        "${CMAKE_CURRENT_BINARY_DIR}/c99_check"
        SOURCES ${C99_TEST_FILE}
        CMAKE_FLAGS
            "-DCMAKE_C_STANDARD=99"
            "-DCMAKE_C_STANDARD_REQUIRED=ON"
            "-DCMAKE_C_EXTENSIONS=OFF"
        COMPILE_DEFINITIONS
            "-std=c99 -pedantic -Werror"
    )
    
    if(NOT C99_SUPPORTED)
        message(FATAL_ERROR "Compiler does not support C99 standard")
    else()
        message(STATUS "✓ C99 compliance verified")
    endif()
endfunction()

# Check for non-C99 constructs in source
function(check_source_c99_compliance SOURCE_FILE)
    file(READ ${SOURCE_FILE} SOURCE_CONTENT)
    
    # Check for common non-C99 patterns
    set(VIOLATIONS "")
    
    # Check for K&R style function definitions
    if(SOURCE_CONTENT MATCHES "\\n[a-zA-Z_][a-zA-Z0-9_]*\\([^)]*\\)\\s*[a-zA-Z_]")
        list(APPEND VIOLATIONS "K&R style function definition detected")
    endif()
    
    # Check for implicit int
    if(SOURCE_CONTENT MATCHES "\\n[^/]*\\bmain\\s*\\(")
        if(NOT SOURCE_CONTENT MATCHES "\\bint\\s+main\\s*\\(")
            list(APPEND VIOLATIONS "Implicit int return type")
        endif()
    endif()
    
    # Check for gets() usage (dangerous and removed in C11)
    if(SOURCE_CONTENT MATCHES "\\bgets\\s*\\(")
        list(APPEND VIOLATIONS "Use of gets() - use fgets() instead")
    endif()
    
    if(VIOLATIONS)
        message(WARNING "C99 compliance issues in ${SOURCE_FILE}:")
        foreach(VIOLATION ${VIOLATIONS})
            message(WARNING "  - ${VIOLATION}")
        endforeach()
    endif()
endfunction()
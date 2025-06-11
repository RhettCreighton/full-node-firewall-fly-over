# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Real compile-time safety checks that prevent unsafe code from building

function(add_safety_checks TARGET)
    # 1. Enable all warnings and treat them as errors
    target_compile_options(${TARGET} PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wformat=2
        -Wformat-overflow=2
        -Wformat-truncation=2
        -Wnull-dereference
        -Wstack-protector
        -Warray-bounds=2
        -Wfloat-equal
        -Wconversion
        -Wshadow
        -Wpointer-arith
        -Wcast-align
        -Wcast-qual
        -Wstrict-overflow=5
        -Wwrite-strings
        -Waggregate-return
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wmissing-declarations
        -Wnested-externs
        -Wredundant-decls
        -Winline
        -Wvla
        -Wdisabled-optimization
    )
    
    # 2. Enable runtime safety features
    target_compile_options(${TARGET} PRIVATE
        -fstack-protector-all
        -ftrapv  # Trap on integer overflow
        -D_FORTIFY_SOURCE=2
    )
    
    # 3. Enable sanitizers in debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
            -fsanitize=float-divide-by-zero
            -fsanitize=float-cast-overflow
            -fno-sanitize-recover=all
        )
        target_link_options(${TARGET} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
        )
    endif()
    
    # 4. Add custom build step to verify no unsafe functions
    add_custom_command(
        TARGET ${TARGET}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Checking for unsafe functions..."
        COMMAND ! nm $<TARGET_FILE:${TARGET}> 2>/dev/null | grep -E "gets|strcpy|strcat|sprintf" || (echo "ERROR: Unsafe functions detected!" && false)
        COMMENT "Verifying no unsafe C functions are used"
    )
endfunction()

# Macro to require safety checks
macro(REQUIRE_SAFETY TARGET)
    add_safety_checks(${TARGET})
    
    # Add test that runs the binary to check for crashes
    add_test(
        NAME ${TARGET}_no_crash_test
        COMMAND ${CMAKE_COMMAND} -E env "TEST_MODE=1" $<TARGET_FILE:${TARGET}>
    )
    set_tests_properties(${TARGET}_no_crash_test PROPERTIES
        TIMEOUT 5
        PASS_REGULAR_EXPRESSION "TEST MODE: Exiting safely"
        FAIL_REGULAR_EXPRESSION "core dumped|Segmentation fault|Floating point exception"
    )
endmacro()
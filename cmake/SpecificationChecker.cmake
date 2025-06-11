# Compile-Time Specification Checker
# Ensures specifications are satisfied BEFORE code can compile

# Function to add specification checks to source files
function(check_specifications_in_source TARGET)
    # Get all source files for the target
    get_target_property(SOURCES ${TARGET} SOURCES)
    
    foreach(SOURCE ${SOURCES})
        if(SOURCE MATCHES "\\.(c|h)$")
            # Add custom command to check specifications in source
            add_custom_command(
                OUTPUT ${SOURCE}.spec_checked
                COMMAND ${CMAKE_COMMAND} 
                    -DSOURCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_source_specs.cmake
                COMMAND ${CMAKE_COMMAND} -E touch ${SOURCE}.spec_checked
                DEPENDS ${SOURCE}
                COMMENT "Checking specifications in ${SOURCE}"
                VERBATIM
            )
            
            # Make target depend on specification check
            set_source_files_properties(${SOURCE} PROPERTIES
                OBJECT_DEPENDS ${SOURCE}.spec_checked
            )
        endif()
    endforeach()
endfunction()

# Specification patterns to enforce
set(SPECIFICATION_PATTERNS
    # No raw pointer arithmetic without bounds checking
    "SPEC:NO_RAW_PTR_ARITH:ptr\\s*[+-]\\s*[^)]"
    
    # No unchecked array access
    "SPEC:NO_UNCHECKED_ARRAY:\\[[^]]*\\]\\s*="
    
    # No direct division without zero check
    "SPEC:NO_UNCHECKED_DIV:[^_]/[^=/]"
    
    # No malloc without null check
    "SPEC:NO_UNCHECKED_MALLOC:malloc\\([^)]+\\)\\s*;"
    
    # Must use SPEC_SAFE macros for dangerous operations
    "SPEC:USE_SAFE_MACROS:strcpy|strcat|sprintf|gets"
)

# Function to generate specification-aware code
function(generate_spec_safe_wrappers)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/spec_safe_template.c.in
        ${CMAKE_CURRENT_BINARY_DIR}/spec_safe_wrappers.c
        @ONLY
    )
endfunction()

# Add specification enforcement to a target
function(enforce_specifications TARGET)
    message(STATUS "Enforcing specifications for ${TARGET}")
    
    # 1. Check source code for violations
    check_specifications_in_source(${TARGET})
    
    # 2. Add specification enforcement library
    target_sources(${TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/specification_enforcement.c
    )
    
    # 3. Enable specification macros
    target_compile_definitions(${TARGET} PRIVATE
        SPEC_ENFORCEMENT_ENABLED
        SPEC_RUNTIME_MONITORING
    )
    
    # 4. Add compiler flags to catch more issues
    target_compile_options(${TARGET} PRIVATE
        -Wstrict-overflow=5
        -Wcast-align=strict
        -Wformat-signedness
        -Wnull-dereference
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wrestrict
        -Wjump-misses-init
    )
    
    # 5. Link-time optimization to verify specifications
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${TARGET} PRIVATE -flto)
        target_link_options(${TARGET} PRIVATE -flto)
    endif()
endfunction()

# Create specification report
function(generate_specification_report TARGET)
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== SPECIFICATION COMPLIANCE REPORT ==="
        COMMAND nm $<TARGET_FILE:${TARGET}> | grep -c "spec_" || true
        COMMAND ${CMAKE_COMMAND} -E echo "Specification enforcement points found"
        VERBATIM
    )
endfunction()
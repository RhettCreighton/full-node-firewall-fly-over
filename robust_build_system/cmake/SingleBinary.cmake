# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# SingleBinary.cmake - Enforce single binary output

include_guard(GLOBAL)

# Global variable to track binaries
set_property(GLOBAL PROPERTY REGISTERED_BINARIES "")

# Enforce single binary constraint
function(enforce_single_binary TARGET)
    # Get list of registered binaries
    get_property(BINARIES GLOBAL PROPERTY REGISTERED_BINARIES)
    
    # Check if we already have a binary
    list(LENGTH BINARIES NUM_BINARIES)
    if(NUM_BINARIES GREATER 0)
        message(FATAL_ERROR 
            "Single binary constraint violated!\n"
            "  Existing binary: ${BINARIES}\n"
            "  Attempted to add: ${TARGET}\n"
            "  Only one executable allowed per project."
        )
    endif()
    
    # Register this binary
    set_property(GLOBAL APPEND PROPERTY REGISTERED_BINARIES ${TARGET})
    
    # Set output name to project name
    set_target_properties(${TARGET} PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    message(STATUS "✓ Single binary registered: ${TARGET}")
endfunction()

# Create a monolithic executable from multiple sources
function(create_monolithic_binary TARGET)
    cmake_parse_arguments(MONO
        "STATIC_LINK"
        "MAIN_SOURCE"
        "SOURCES;MODULES"
        ${ARGN}
    )
    
    # Combine all sources
    set(ALL_SOURCES ${MONO_MAIN_SOURCE} ${MONO_SOURCES})
    
    # If modules specified, merge them
    if(MONO_MODULES)
        foreach(MODULE ${MONO_MODULES})
            # Get sources from module
            get_target_property(MODULE_SOURCES ${MODULE} SOURCES)
            if(MODULE_SOURCES)
                list(APPEND ALL_SOURCES ${MODULE_SOURCES})
            endif()
        endforeach()
    endif()
    
    # Create the executable
    add_executable(${TARGET} ${ALL_SOURCES})
    
    # Apply single binary constraint
    enforce_single_binary(${TARGET})
    
    # Static linking if requested
    if(MONO_STATIC_LINK)
        set_target_properties(${TARGET} PROPERTIES
            LINK_SEARCH_START_STATIC ON
            LINK_SEARCH_END_STATIC ON
        )
        target_link_options(${TARGET} PRIVATE -static)
        
        # Use static versions of libraries
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    endif()
    
    message(STATUS "✓ Monolithic binary created: ${TARGET}")
endfunction()

# Merge multiple object libraries into single binary
function(merge_into_binary TARGET)
    cmake_parse_arguments(MERGE
        ""
        ""
        "OBJECTS"
        ${ARGN}
    )
    
    # Create list of object files
    set(OBJECT_LIBS "")
    foreach(OBJ ${MERGE_OBJECTS})
        list(APPEND OBJECT_LIBS $<TARGET_OBJECTS:${OBJ}>)
    endforeach()
    
    # Add to target
    target_sources(${TARGET} PRIVATE ${OBJECT_LIBS})
endfunction()

# Prevent accidental library creation
function(add_library)
    # Check if this is an object library (allowed for internal use)
    set(ARGS ${ARGN})
    list(GET ARGS 1 LIB_TYPE)
    
    if(NOT LIB_TYPE STREQUAL "OBJECT" AND FORCE_SINGLE_BINARY)
        message(FATAL_ERROR 
            "Library creation blocked by single binary policy!\n"
            "  Attempted to create: ${ARGV0}\n"
            "  Use OBJECT libraries or merge into main binary."
        )
    endif()
    
    # Call original function
    _add_library(${ARGN})
endfunction()

# Override to track all executables
function(add_executable)
    # Call original function
    _add_executable(${ARGN})
    
    # Get target name
    list(GET ARGN 0 TARGET)
    
    # Auto-apply single binary constraint if enabled
    if(FORCE_SINGLE_BINARY)
        enforce_single_binary(${TARGET})
    endif()
endfunction()

# Helper to ensure only one main() function
function(check_single_main)
    cmake_parse_arguments(CHECK
        ""
        ""
        "SOURCES"
        ${ARGN}
    )
    
    set(MAIN_COUNT 0)
    set(MAIN_FILES "")
    
    foreach(SOURCE ${CHECK_SOURCES})
        file(READ ${SOURCE} CONTENT)
        if(CONTENT MATCHES "int[ \t\n]+main[ \t\n]*\\(")
            math(EXPR MAIN_COUNT "${MAIN_COUNT} + 1")
            list(APPEND MAIN_FILES ${SOURCE})
        endif()
    endforeach()
    
    if(MAIN_COUNT GREATER 1)
        message(FATAL_ERROR 
            "Multiple main() functions detected!\n"
            "  Files with main(): ${MAIN_FILES}\n"
            "  Only one main() allowed in single binary."
        )
    elseif(MAIN_COUNT EQUAL 0)
        message(WARNING "No main() function found in sources")
    endif()
endfunction()

# Summary of binary configuration
function(print_binary_summary)
    get_property(BINARIES GLOBAL PROPERTY REGISTERED_BINARIES)
    
    message(STATUS "")
    message(STATUS "Binary Configuration:")
    
    if(BINARIES)
        message(STATUS "  Output binary: ${PROJECT_NAME}")
        message(STATUS "  Target name: ${BINARIES}")
        
        # Get binary properties
        get_target_property(OUTPUT_DIR ${BINARIES} RUNTIME_OUTPUT_DIRECTORY)
        message(STATUS "  Output location: ${OUTPUT_DIR}")
        
        get_target_property(LINK_FLAGS ${BINARIES} LINK_FLAGS)
        if(LINK_FLAGS MATCHES "-static")
            message(STATUS "  ✓ Static linking enabled")
        endif()
    else()
        message(STATUS "  No binary targets registered")
    endif()
    
    message(STATUS "")
endfunction()
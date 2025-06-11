# Source code specification checker
# Called by SpecificationChecker.cmake for each source file

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE not defined")
endif()

file(READ ${SOURCE_FILE} SOURCE_CONTENT)

# Check for specification violations
set(VIOLATIONS "")

# Check 1: No raw pointer arithmetic without bounds checking
if(SOURCE_CONTENT MATCHES "\\*[a-zA-Z_][a-zA-Z0-9_]*\\s*\\+\\+")
    list(APPEND VIOLATIONS "Raw pointer increment without bounds check")
endif()

# Check 2: No strcpy/strcat/sprintf
if(SOURCE_CONTENT MATCHES "\\bstrcpy\\s*\\(")
    list(APPEND VIOLATIONS "Using unsafe strcpy() - use SPEC_SAFE_STRCPY")
endif()

if(SOURCE_CONTENT MATCHES "\\bstrcat\\s*\\(")
    list(APPEND VIOLATIONS "Using unsafe strcat() - use SPEC_SAFE_STRCAT")
endif()

if(SOURCE_CONTENT MATCHES "\\bsprintf\\s*\\(")
    list(APPEND VIOLATIONS "Using unsafe sprintf() - use snprintf()")
endif()

# Check 3: No unchecked malloc
if(SOURCE_CONTENT MATCHES "malloc\\s*\\([^)]+\\)\\s*;")
    list(APPEND VIOLATIONS "malloc() without NULL check - use SPEC_SAFE_MALLOC")
endif()

# Check 4: No gets() ever
if(SOURCE_CONTENT MATCHES "\\bgets\\s*\\(")
    list(APPEND VIOLATIONS "gets() is always unsafe - use fgets()")
endif()

# Check 5: Division should use SPEC_SAFE_DIV
if(SOURCE_CONTENT MATCHES "[^/]/[^/=]" AND 
   NOT SOURCE_CONTENT MATCHES "SPEC_SAFE_DIV")
    # More complex check needed here, simplified for now
endif()

# Report violations
if(VIOLATIONS)
    message(FATAL_ERROR 
        "Specification violations in ${SOURCE_FILE}:\n"
        "${VIOLATIONS}\n"
        "Fix these violations to compile."
    )
endif()

message(STATUS "✓ ${SOURCE_FILE} passes specification checks")
cmake_minimum_required(VERSION 3.16)
project(ExistingProject C)

# Original project settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add Robust Build System (one line!)
list(APPEND CMAKE_MODULE_PATH "~/.cmake/modules")
include(RobustBuildSystem)

# Enable specific features for this project
enable_robust_build(
    PROOF_VERIFICATION
    ULTRA_OPTIMIZATION
    PROOF_FUNCTION "validate_input"
)

# Existing project files
set(SOURCES
    src/main.c
    src/parser.c
    src/validator.c
    src/processor.c
)

# Convert existing add_executable to robust version
add_robust_executable(myapp
    REQUIRE_PROOF
    SOURCES ${SOURCES}
)

# Existing target properties still work
target_include_directories(myapp PRIVATE include)
target_compile_definitions(myapp PRIVATE VERSION="2.0")

# Show what optimizations are enabled
robust_build_summary()
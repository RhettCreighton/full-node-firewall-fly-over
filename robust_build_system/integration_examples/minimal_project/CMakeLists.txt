cmake_minimum_required(VERSION 3.16)
project(MinimalExample C)

# Add the Robust Build System module path
list(APPEND CMAKE_MODULE_PATH "~/.cmake/modules")

# Include and enable Robust Build System
include(RobustBuildSystem)
robust_c99_project()

# Simple executable with proof verification
add_robust_executable(minimal
    REQUIRE_PROOF
    PROOF_FUNCTION "critical_function"
    SOURCES src/main.c
)
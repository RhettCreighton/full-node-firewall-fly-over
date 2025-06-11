cmake_minimum_required(VERSION 3.16)
project(RobustLibrary C)

# Include Robust Build System
list(APPEND CMAKE_MODULE_PATH "~/.cmake/modules")
include(RobustBuildSystem)

# Enable for libraries too
enable_robust_build(
    ULTRA_OPTIMIZATION
    NATIVE_ARCH
    BUILD_PROFILING
)

# Create optimized static library
add_library(robust_math STATIC
    src/vector_ops.c
    src/matrix_ops.c
    src/fast_math.c
)

# Apply optimizations to library
if(COMMAND enable_ultimate_optimization)
    enable_ultimate_optimization(robust_math)
endif()

# Create test executable with proof
add_robust_executable(test_math
    REQUIRE_PROOF
    PROOF_FUNCTION "test_verification"
    SOURCES test/test_main.c
)
target_link_libraries(test_math robust_math)

# Install rules
install(TARGETS robust_math
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(FILES include/robust_math.h DESTINATION include)
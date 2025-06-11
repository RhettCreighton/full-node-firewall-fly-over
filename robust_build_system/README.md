# Robust C99 CMake Build System

A professional-grade build system that enforces compile-time proofs, C99 compliance, and comprehensive safety checks.

## Key Features

### 1. Compile-Time Proof Verification
- **GDB-based verification**: Proves functions are called before allowing compilation
- **Symbol verification**: Ensures required functions exist and forbidden functions don't
- **Invariant checking**: Verifies program invariants at build time
- **Output validation**: Checks program output matches expectations

### 2. C99 Standard Compliance
- Enforces strict C99 standard (`-std=c99 -pedantic`)
- Prevents non-C99 constructs
- Checks for dangerous functions (gets, strcpy, etc.)
- Validates proper function prototypes

### 3. Comprehensive Safety
- Stack protection (`-fstack-protector-strong`)
- FORTIFY_SOURCE for buffer overflow protection
- Address sanitizer support (Debug builds)
- Integer overflow detection
- Memory safety wrappers

### 4. Build Optimization
- Profile-guided optimization support
- Link-time optimization (LTO)
- Unity builds for faster compilation
- Parallel build configuration
- ccache integration

### 5. Single Binary Enforcement
- Prevents multiple executables
- Ensures clean, simple deployment
- Monolithic binary support
- Static linking options

## Quick Start

### Initialize a New Project

```bash
chmod +x init_project.sh
./init_project.sh MySecureApp ~/projects/my_secure_app
cd ~/projects/my_secure_app
./build.sh
```

### Manual Setup

1. Copy the `cmake/` directory to your project
2. Create a CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyApp VERSION 1.0.0 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompileTimeProof)
include(C99Compliance)
include(BuildOptimization)
include(SafetyChecks)
include(SingleBinary)

add_verified_executable(MyApp
    SOURCES src/main.c
    PROOF_FUNCTION required_function
    EXPECTED_OUTPUT "Expected output"
    INVARIANTS required_function
)

apply_c99_compliance(MyApp)
apply_safety_checks(MyApp)
```

## Compile-Time Proofs

### Basic Proof

```cmake
add_verified_executable(MyApp
    SOURCES src/main.c
    PROOF_FUNCTION firefox_draw_pixel
    EXPECTED_OUTPUT "Drawing pixel"
)
```

This will:
1. Compile a test binary
2. Run it and check for "Drawing pixel" in output
3. Only allow real compilation if proof passes

### Advanced Proofs

```cmake
add_verified_executable(SecureApp
    SOURCES ${SOURCES}
    
    # Function that must be called
    PROOF_FUNCTION critical_init
    EXPECTED_OUTPUT "Initialized"
    
    # Multiple invariants to verify
    INVARIANTS
        validate_input      # Must validate input
        check_bounds       # Must check bounds
        log_operation      # Must log operations
    
    # Required security functions
    REQUIRED_SYMBOLS
        __stack_chk_fail   # Stack protection
        __fortify_fail     # Buffer protection
    
    # Banned unsafe functions
    FORBIDDEN_SYMBOLS
        strcpy             # Use safe_strncpy
        gets               # Use fgets
        sprintf            # Use snprintf
)
```

## Safety Features

### Memory Safety

```c
// Automatically included safe functions
void *safe_malloc(size_t size);
void *safe_calloc(size_t nmemb, size_t size);
char *safe_strncpy(char *dest, const char *src, size_t n);

// Runtime assertions
BOUNDS_CHECK(index, size);
NULL_CHECK(pointer);
INVARIANT(condition);
```

### Integer Safety

```c
// Safe arithmetic with overflow checking
int result;
SAFE_ADD(a, b, result);         // Aborts on overflow
SAFE_MULTIPLY(x, y, result);     // Aborts on overflow
```

## Build Options

### CMake Options

- `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo, MinSizeRel
- `ENABLE_PROOF_SYSTEM`: Enable compile-time proofs (ON)
- `ENABLE_ALL_SAFETY`: Enable all safety checks (ON)
- `ENABLE_LTO`: Link-time optimization (ON)
- `ENABLE_NATIVE_ARCH`: Optimize for native CPU (OFF)
- `FORCE_SINGLE_BINARY`: Enforce single binary (ON)

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```
Enables:
- AddressSanitizer
- Full debug symbols
- All strict warnings
- Runtime assertions

### Release Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```
Enables:
- Full optimizations (-O3)
- Link-time optimization
- Binary stripping
- FORTIFY_SOURCE=2

## Static Analysis

Automatic integration with:
- clang-tidy (if available)
- cppcheck (if available)
- Compiler warnings as errors

## Example: Firefox Pixel App

See `example/` directory for a complete application that:
- Must draw Firefox pixels (compile-time verified)
- Uses safe memory functions
- Validates all bounds
- Prevents integer overflow
- Produces a single, optimized binary

## Advanced Usage

### Custom Proof Functions

```cmake
# Require multiple functions to be called
require_function_called(MyApp 
    process_data "Processing"
)

# Add memory safety requirements
add_memory_safety_proof(MyApp)

# Add fuzzing support
add_fuzzing_support(MyApp ${SOURCES})
```

### Profile-Guided Optimization

```bash
# Step 1: Generate profile
cmake -DENABLE_PGO_GENERATE=ON ..
make
./myapp  # Run typical workload

# Step 2: Use profile
cmake -DENABLE_PGO_USE=ON ..
make
```

## Philosophy

This build system enforces the principle: **"Make it impossible to build unsafe code"**

- If it compiles, it's C99 compliant
- If it compiles, required functions are called
- If it compiles, unsafe functions aren't used
- If it compiles, safety checks are in place

## License

Apache-2.0 License. See LICENSE file for details.
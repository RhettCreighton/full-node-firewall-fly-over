# Compile-Time GDB Proof System

A build system that **REFUSES** to compile unless GDB can prove your code is safe!

## What It Does

This module provides compile-time verification that critical safety functions are called in your code. The build will **FAIL** if GDB cannot prove that your binary:

1. Initializes the aircraft manager properly
2. Enables bounds checking on arrays
3. Enables NULL pointer checking
4. Establishes the no-coredump guarantee

## How It Works

1. You add special proof functions to your code
2. CMake compiles your binary with debug symbols
3. GDB runs the binary and verifies all proof functions are called
4. If verification fails, the build is **BLOCKED**
5. If verification passes, you get a coredump-proof binary!

## Quick Start

### 1. In your CMakeLists.txt:

```cmake
# Enable GDB proof system
add_executable(sky_combat_multiplayer 
    src/sky_combat_multiplayer.c
    # ... other sources ...
)

# This line makes the build REQUIRE safety proofs!
REQUIRE_NO_COREDUMP_GUARANTEE(sky_combat_multiplayer)
```

### 2. In your C code:

```c
#include "gdb_proof.h"

int main() {
    // These calls are REQUIRED - build fails without them!
    gdb_proof_init_aircraft_manager();
    gdb_proof_bounds_check_enabled();
    gdb_proof_null_check_enabled();
    gdb_proof_no_coredump_guarantee();
    
    // ... rest of your game ...
}
```

### 3. Build:

```bash
mkdir build && cd build
cmake ..
make

# If any proof function is missing:
# ❌ BUILD BLOCKED: Safety verification failed!

# If all proofs pass:
# ✅ Compile-time verification PASSED!
```

## The Safety Functions

### `gdb_proof_init_aircraft_manager()`
Proves the aircraft manager is properly initialized with bounds checking.

### `gdb_proof_bounds_check_enabled()`
Proves array bounds checking is active to prevent buffer overflows.

### `gdb_proof_null_check_enabled()`
Proves NULL pointer checking prevents segmentation faults.

### `gdb_proof_no_coredump_guarantee()`
The ultimate proof - called when ALL safety systems are verified!

## Advanced Usage

### Custom Function Verification

```cmake
# Require any function to be called
require_function_proof(my_target "critical_init_function")
```

### Conditional Verification

```cmake
# Only in Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    REQUIRE_NO_COREDUMP_GUARANTEE(my_target)
endif()
```

### CI/CD Integration

```yaml
# GitHub Actions example
- name: Build with Safety Verification
  run: |
    cmake -B build -DENABLE_GDB_PROOF=ON
    cmake --build build
    # Build fails if safety not proven!
```

## How the Verification Works

1. **Compile Phase**: Normal compilation with `-g` flag
2. **Verification Phase**: 
   ```bash
   gdb -batch -x verify_script.gdb your_binary
   ```
3. **Proof Check**: GDB sets breakpoints on all required functions
4. **Execution**: Binary is run under GDB
5. **Result**: 
   - ✅ All breakpoints hit = Build continues
   - ❌ Any breakpoint missed = Build FAILS

## Example Output

### Successful Build:
```
=== GDB Safety Verification for sky_combat ===
Running GDB verification...
[GDB PROOF] Aircraft manager initialization verified
[GDB PROOF] Array bounds checking is enabled
[GDB PROOF] NULL pointer checking is enabled
[GDB PROOF] ✓ NO COREDUMP GUARANTEE ESTABLISHED ✓

✅ Compile-time verification PASSED!
This binary is guaranteed not to coredump.
```

### Failed Build:
```
=== GDB Safety Verification for bad_binary ===
❌ FAILED: Aircraft manager not initialized
❌ FAILED: Bounds checking not enabled

❌ BUILD BLOCKED: Safety verification failed!
Your binary must call these functions:
  - gdb_proof_init_aircraft_manager()
  - gdb_proof_bounds_check_enabled()
  - gdb_proof_null_check_enabled()
  - gdb_proof_no_coredump_guarantee()
```

## Philosophy

> "If you can't prove it's safe at compile time, it shouldn't compile!"

This system enforces safety by making it **impossible** to build unsafe code. No more "I'll add error checking later" - the build system demands proof NOW!

## Integration with Sky Combat

Sky Combat uses this system to guarantee:
- No array out-of-bounds access (max 16 aircraft)
- No NULL pointer dereferences 
- No uninitialized aircraft manager usage
- No coredumps during gameplay!

## Troubleshooting

### "Binary not found"
- Make sure target is built before verification
- Check CMake target name matches

### "Verification keeps failing"
- Ensure ALL four proof functions are called
- Check they're called in main execution path
- Not in unreachable code or after early return

### "GDB not found"
```bash
# Install GDB
sudo apt install gdb  # Debian/Ubuntu
sudo yum install gdb  # RedHat/Fedora
```

## License

Apache-2.0 - Because safety should be free!
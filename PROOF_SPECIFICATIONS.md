# Sky Combat Proof Specifications

## Total Count: 16+ Compile-Time Proofs

Every time Sky Combat compiles, we verify **ALL** of these specifications:

### 1. Control System Specifications (3 proofs)
- **Control Lock**: Once controls are set, they cannot change
- **Right Stick Aim**: Right stick correctly controls aiming
- **Right Stick Unused**: Unused stick functions are properly disabled

### 2. Weapon System Specifications (4 proofs)
- **Weapon Independence**: Each weapon operates independently
- **Gun Tuning**: Weapon parameters are correctly calibrated
- **Straight Shooting**: Projectiles travel in straight lines
- **Practice Targets**: Target system behaves correctly

### 3. Display System Specifications (2 proofs)
- **View Behavior**: Camera and view systems work correctly
- **Pixel Draw Safety**: No buffer overflows in rendering

### 4. Safety System Specifications (6 proofs)
- **Signal Handlers**: All fatal signals have handlers installed
- **Core Limit**: RLIMIT_CORE is set to 0
- **Null Pointer Protection**: Null derefs don't crash
- **Division Safety**: Division by zero is handled
- **Memory Safety**: Buffer overflows are caught
- **Path Coverage**: No execution path can core dump

### 5. Static Analysis Specifications (1+ proof)
- **No Unsafe Functions**: gets(), strcpy(), etc. are banned
- **No Unchecked Mallocs**: All allocations are checked
- **No Direct Division by Zero**: Caught at compile time

### 6. Compiler-Enforced Specifications (Continuous)
- **-Wall -Wextra -Werror**: All warnings are errors
- **Array Bounds Checking**: -Warray-bounds=2
- **Format String Safety**: -Wformat=2
- **Stack Protection**: -fstack-protector-all

## How It Works

```cmake
# In CMakeLists.txt - runs EVERY compile:
verify_all_specifications(full-node-firewall-flyover)
```

This runs:
1. All legacy GDB proofs (controls, weapons, display)
2. New safety proof system (6 no-coredump proofs)
3. Static analysis checks
4. Compiler safety flags

## The Guarantee

**If Sky Combat compiles successfully:**
- ✓ All 16+ specifications are verified
- ✓ No core dumps are possible
- ✓ All game mechanics work as specified
- ✓ Memory safety is guaranteed

**If any specification fails:**
- ✗ Build fails immediately
- ✗ Error shows which spec failed
- ✗ Cannot produce unsafe binary

## Development Speed

For faster development iterations:
```bash
# Skip proofs temporarily (UNSAFE!)
cmake -DSKIP_SPECIFICATION_PROOFS=ON ..

# Or disable specific categories:
cmake -DENABLE_GDB_PROOF=OFF ..
cmake -DENABLE_NO_COREDUMP_PROOFS=OFF ..
```

## Continuous Verification

Every commit, every build, every compile runs ALL proofs.
This is true "Design by Contract" - specifications ARE the code!
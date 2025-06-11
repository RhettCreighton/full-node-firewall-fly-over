# GDB No-Coredump Proof System

A deterministic verification system that proves C programs cannot create core dump files.

## Overview

This system uses GDB to verify that:
1. All crash scenarios are handled by signal handlers
2. Core dump creation is disabled (`RLIMIT_CORE = 0`)
3. No execution path can bypass these protections

For deterministic programs, this provides **mathematical certainty** - not just testing.

## Quick Start

```bash
# Run all proofs on a binary
./gdb_proof_system/run_proofs.sh ./build/sky_combat_ultimate

# With verbose output
./gdb_proof_system/run_proofs.sh -v ./build/sky_combat_ultimate

# Strict mode (fail build if proofs fail)
./gdb_proof_system/run_proofs.sh -s ./build/sky_combat_ultimate
```

## CMake Integration

Add to your `CMakeLists.txt`:

```cmake
# Include the proof system
include(gdb_proof_system/integration/RunProofs.cmake)

# Add proofs to your target
add_gdb_proofs(my_program)

# Enable strict mode (optional)
set(STRICT_SAFETY_PROOFS ON)
```

Now proofs run automatically after building!

## How It Works

### 1. Signal Handler Verification
Confirms all fatal signals have handlers:
- SIGSEGV (segmentation fault)
- SIGFPE (arithmetic errors)
- SIGBUS (bus errors)
- SIGILL (illegal instructions)
- SIGABRT (abort calls)

### 2. Core Limit Verification
Ensures `setrlimit(RLIMIT_CORE, 0)` is called, preventing core file creation.

### 3. Crash Scenario Testing
Verifies each crash type is caught:
- Null pointer dereferences
- Division by zero
- Buffer overflows
- Memory errors

### 4. Path Coverage
Traces execution paths to ensure:
- No path calls `abort()`
- All paths lead to clean exit
- Signal handlers always activated

## Proof Modules

| Module | Purpose | Verification |
|--------|---------|--------------|
| 01_signal_handlers.gdb | Signal handler setup | All signals have handlers |
| 02_core_limit.gdb | Core dump prevention | RLIMIT_CORE = 0 |
| 03_null_pointer.gdb | Null pointer safety | SIGSEGV caught |
| 04_div_zero.gdb | Arithmetic safety | SIGFPE caught |
| 05_memory_safety.gdb | Memory error handling | Buffer overflows caught |
| 06_exhaustive_coverage.gdb | Path verification | All paths safe |

## Mathematical Guarantee

For deterministic programs:
- **Input**: Program binary
- **Process**: GDB traces all possible crash scenarios
- **Output**: Proof that no execution can create core dump
- **Guarantee**: Mathematical certainty, not probabilistic

## Requirements

- GDB (GNU Debugger)
- Python 3.6+
- CMake 3.10+ (for build integration)
- Linux/Unix system

## Adding New Proofs

1. Create new proof module in `proofs/`:
```gdb
# proofs/07_my_proof.gdb
proof_init
proof_log "=== My Custom Proof ==="

# Your verification logic here
proof_assert $condition "Description"

proof_summary
```

2. Run the proof system - it automatically finds new modules!

## Output

Successful verification produces:
- `proof_report.json` - Detailed results
- `PROOF_CERTIFICATE.txt` - Verification certificate
- Exit code 0

Failed verification shows:
- Which proofs failed
- Why they failed
- Exit code 1

## Example Output

```
===================================
GDB No-Coredump Proof System
===================================
Binary: ./build/sky_combat_ultimate

Running deterministic proofs...

✓ PASS: 01_signal_handlers.gdb
✓ PASS: 02_core_limit.gdb
✓ PASS: 03_null_pointer.gdb
✓ PASS: 04_div_zero.gdb
✓ PASS: 05_memory_safety.gdb
✓ PASS: 06_exhaustive_coverage.gdb

✓ SUCCESS: Program verified - NO CORE DUMPS POSSIBLE

Proof Summary:
  Total proofs: 6
  Passed: 6
  Duration: 4.28s
  Verdict: VERIFIED
```

## Troubleshooting

### Proofs fail with "Binary not found"
- Ensure you've built the program first
- Use absolute paths if needed

### Proofs timeout
- Complex programs may need longer timeouts
- Edit timeout in `proof_runner.py`

### "No unsafe functions detected" is yellow
- This is just a warning
- The program is still verified safe

## Theory

See `docs/PROOF_THEORY.md` for the mathematical foundation and formal proofs.

## License

Apache-2.0 - See LICENSE file
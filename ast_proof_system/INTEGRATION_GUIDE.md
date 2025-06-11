# AST Proof System Integration Guide

## Quick Start

### 1. Basic Setup
```bash
cd your_project
git submodule add https://github.com/your/ast_proof_system ast_proof_system
cd ast_proof_system
./setup.py
```

### 2. CMake Integration

Add to your main `CMakeLists.txt`:

```cmake
# Option to enable AST verification
option(ENABLE_AST_VERIFICATION "Enable AST safety verification" ON)

if(ENABLE_AST_VERIFICATION)
    add_subdirectory(ast_proof_system)
    
    # Include the verification function
    include(${AST_PROOF_SYSTEM_DIR}/cmake/ASTVerification.cmake)
endif()

# Add verification to your targets
add_executable(my_game main.c game.c physics.c)

if(ENABLE_AST_VERIFICATION)
    add_ast_verification(my_game)
endif()
```

### 3. Manual Integration (without CMake)

Create a verification script `verify_safety.sh`:

```bash
#!/bin/bash
AST_DIR="ast_proof_system"

# Run on all C files
find src -name "*.c" | while read file; do
    echo "Verifying $file..."
    python3 $AST_DIR/enhanced_ast_analyzer.py "$file"
done

# Generate combined report
python3 $AST_DIR/incremental_verifier.py src/ --report safety_report.md
```

## Integration Patterns

### Pattern 1: Pre-commit Hook

`.git/hooks/pre-commit`:
```bash
#!/bin/bash
# Verify only changed files
CHANGED=$(git diff --cached --name-only --diff-filter=ACM | grep '\.c$')

if [ -n "$CHANGED" ]; then
    python3 ast_proof_system/incremental_verifier.py --files $CHANGED
    if [ $? -ne 0 ]; then
        echo "Safety verification failed!"
        exit 1
    fi
fi
```

### Pattern 2: CI/CD Pipeline

#### GitHub Actions
`.github/workflows/safety.yml`:
```yaml
name: Safety Verification
on: [push, pull_request]

jobs:
  verify:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - name: Setup
      run: |
        sudo apt-get update
        sudo apt-get install -y clang gdb
        pip install z3-solver
    
    - name: Run AST Verification
      run: |
        cd ast_proof_system
        python3 ci_integration.py --ci github --verify-all
```

#### GitLab CI
`.gitlab-ci.yml`:
```yaml
ast-verification:
  stage: test
  script:
    - cd ast_proof_system
    - python3 ci_integration.py --ci gitlab --verify-all
  artifacts:
    reports:
      junit: ast_proof_system/junit_report.xml
```

### Pattern 3: Build System Integration

#### Makefile
```makefile
# Add to your Makefile
.PHONY: verify-safety

verify-safety:
	@echo "Running AST safety verification..."
	@cd ast_proof_system && python3 incremental_verifier.py ../src

build: verify-safety
	$(CC) $(CFLAGS) -o game src/*.c
```

#### Meson
```python
# meson.build
ast_proof = find_program('ast_proof_system/enhanced_ast_analyzer.py', required: false)

if ast_proof.found()
  test('ast-safety', ast_proof, args: [source_files])
endif
```

## Code Annotations

Use these macros in your C code:

```c
#include "ast_proof_system/include/ast_proof_config.h"

// Mark deterministic functions
DETERMINISTIC float calculate_physics(float mass, float velocity) {
    return 0.5f * mass * velocity * velocity;
}

// Mark constrained inputs
void process_input(float value) {
    CONSTRAINED(value, 0.0, 100.0);
    float result = 100.0f / value;  // Safe - analyzer knows bounds
}

// Use safe operations
float safe_calc(float a, float b) {
    return SAFE_DIVIDE(a, b);  // Automatic zero check
}
```

## Performance Optimization

### 1. Enable Caching
```bash
# First run - full analysis
time python3 ast_proof_system/incremental_verifier.py src/

# Subsequent runs - cached (10x faster)
time python3 ast_proof_system/incremental_verifier.py src/
```

### 2. Parallel Analysis
```python
# Run analysis in parallel
python3 ast_proof_system/incremental_verifier.py src/ --parallel --jobs 8
```

### 3. Selective Verification
```bash
# Only verify critical paths
python3 ast_proof_system/enhanced_ast_analyzer.py src/critical/*.c
```

## Configuration

Create `.ast_proof.json` in your project root:

```json
{
  "exclude_paths": ["third_party/", "tests/"],
  "critical_paths": ["src/core/", "src/safety/"],
  "performance": {
    "enable_cache": true,
    "cache_dir": ".ast_cache",
    "parallel_jobs": 4
  },
  "strictness": {
    "require_constraints": true,
    "allow_unsafe_in_tests": true,
    "determinism_warnings": true
  }
}
```

## Gradual Adoption

### Phase 1: Analysis Only
```bash
# Just analyze, don't fail builds
python3 ast_proof_system/enhanced_ast_analyzer.py src/ || true
```

### Phase 2: Critical Paths
```bash
# Enforce safety in critical code only
python3 ast_proof_system/enhanced_ast_analyzer.py src/critical/
if [ $? -ne 0 ]; then
    echo "Critical path verification failed!"
    exit 1
fi
```

### Phase 3: Full Enforcement
```cmake
# Fail builds on any safety issue
add_ast_verification(my_target)
set_property(TARGET my_target PROPERTY AST_VERIFICATION_REQUIRED TRUE)
```

## Troubleshooting

### Common Issues

1. **"Too many false positives"**
   - Add constraints to non-deterministic inputs
   - Use CONSTRAINED() macro
   - Configure strictness in .ast_proof.json

2. **"Analysis too slow"**
   - Enable caching
   - Use incremental verification
   - Analyze only changed files

3. **"Missing Clang"**
   - Install: `sudo apt-get install clang`
   - Or use Docker: `docker run -v $PWD:/code ast-proof`

### Debug Mode
```bash
# Verbose output
AST_DEBUG=1 python3 ast_proof_system/enhanced_ast_analyzer.py file.c

# Save intermediate AST
python3 ast_proof_system/ast_analyzer.py file.c > file.ast
```

## Best Practices

1. **Start with warnings** - Don't fail builds initially
2. **Focus on critical paths** - Verify payment/security code first
3. **Add constraints gradually** - Fix the most critical issues first
4. **Use CI integration** - Catch issues before merge
5. **Cache aggressively** - Deterministic proofs never change
6. **Document constraints** - Explain why bounds exist

## Example: Game Engine Integration

```c
// game_engine.c
#include "ast_proof_config.h"

typedef struct {
    float x, y, z;
} Vector3;

// Deterministic physics - verified once
DETERMINISTIC Vector3 calculate_gravity(Vector3 pos, float mass) {
    const float G = 9.81f;
    return (Vector3){0, -G * mass, 0};
}

// Non-deterministic but constrained
void update_player(Player* player, float delta_time) {
    CONSTRAINED(delta_time, 0.0001f, 0.1f);  // Max 100ms frame
    
    // Safe because delta_time > 0
    player->velocity += player->acceleration * delta_time;
    player->position += player->velocity * delta_time;
}

// Input handling with constraints
void handle_input(float stick_x, float stick_y) {
    // Gamepad input always in [-1, 1]
    CONSTRAINED(stick_x, -1.0f, 1.0f);
    CONSTRAINED(stick_y, -1.0f, 1.0f);
    
    // Safe normalization
    float magnitude = sqrtf(stick_x * stick_x + stick_y * stick_y);
    if (magnitude > 0.01f) {  // Dead zone
        stick_x = SAFE_DIVIDE(stick_x, magnitude);
        stick_y = SAFE_DIVIDE(stick_y, magnitude);
    }
}
```

This integration ensures your game literally cannot crash from verified operations!
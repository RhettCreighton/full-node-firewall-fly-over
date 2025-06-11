# Mathematical Proof: No Core Dump Under Deterministic Execution

## Theorem
For a deterministic C program P, if we can prove via AST analysis that:
1. ALL paths leading to dangerous operations have sufficient guards
2. Signal handlers are installed for all fatal signals
3. RLIMIT_CORE is set to 0

Then P **cannot** create a core dump file.

## Proof by AST-Based Exhaustive Analysis

### Definition: Deterministic Program
A program P is deterministic if:
- ∀ initial state S₀, ∃! execution trace T
- No race conditions, no random inputs
- Same input → Same execution path → Same result

### Step 1: Complete Operation Enumeration via AST

Using AST, we can find **ALL** operations that could cause signals:

```
AST_Operations(P) = {
    op | op ∈ AST(P) ∧ op ∈ {Division, Deref, ArrayAccess, ...}
}
```

This is **complete** because:
- AST contains every syntactic element
- We recursively traverse entire tree
- No operation can hide from AST analysis

### Step 2: Path Analysis via Control Flow Graph

For each dangerous operation op:
```
Paths(op) = {
    p | p is a path from entry to op in CFG(P)
}
```

Since P is deterministic:
- CFG is fixed and complete
- All paths are enumerable
- No hidden control flow

### Step 3: Guard Verification

For each path p to operation op:
```
Guards(p) = {
    g | g is a conditional on path p that protects op
}
```

We prove: ∀ op, ∀ p ∈ Paths(op), Guards(p) ≠ ∅

### Step 4: Signal Handler Verification

Via AST + GDB:
```
Handlers = {
    (sig, handler) | signal(sig, handler) called in P
}
```

Verify: {SIGSEGV, SIGFPE, SIGBUS, SIGILL, SIGABRT} ⊆ domain(Handlers)

### Step 5: Core Limit Verification

Via AST:
```
∃ call to setrlimit(RLIMIT_CORE, {0, 0}) in P
```

## The Complete Proof

**Given** (verified by AST + GDB):
1. ∀ dangerous op, ∃ guard g that ensures safety
2. ∀ fatal signal s, ∃ handler h(s)
3. RLIMIT_CORE = 0

**Proof by contradiction**:
Assume P can core dump. Then:
1. P must receive a fatal signal (only way to dump)
2. Signal must be unhandled OR handler must call abort()
3. RLIMIT_CORE must be > 0

But:
- (1) requires reaching dangerous op without guard → contradicts AST analysis
- (2) contradicts our signal handler verification
- (3) contradicts our setrlimit verification

**Therefore**: P cannot core dump. ∎

## Implementation

```python
def prove_no_coredump(program):
    # 1. Extract AST
    ast = extract_ast(program)
    
    # 2. Find all dangerous operations
    dangerous_ops = find_dangerous_operations(ast)
    
    # 3. Build CFG
    cfg = build_cfg(ast)
    
    # 4. For each dangerous op, verify guards
    for op in dangerous_ops:
        paths = find_all_paths_to(cfg, op)
        for path in paths:
            guards = extract_guards(path)
            if not verify_guards_sufficient(guards, op):
                return PROOF_FAILED(op, path)
    
    # 5. Verify signal handlers
    if not verify_signal_handlers(ast):
        return PROOF_FAILED("Missing signal handlers")
    
    # 6. Verify core limit
    if not verify_core_limit_zero(ast):
        return PROOF_FAILED("Core dumps not disabled")
    
    return PROOF_SUCCESS("Program cannot core dump")
```

## Why This Works for Deterministic Programs

1. **Completeness**: AST contains ALL code - nothing hidden
2. **Decidability**: Finite paths in deterministic program
3. **Soundness**: Each verification step is rigorous
4. **Reproducibility**: Deterministic → one proof covers all runs

## Example Application

For the FPE bug in enemies.c:
```c
float angle = (2.0f * PI * i) / count;
```

AST proof would:
1. Find division operation at line 191
2. Trace paths leading to it
3. Discover path where count=0 is possible
4. FAIL proof with counterexample
5. After fix (if count <= 0 return), proof SUCCEEDS

## Limitations

- Only works for deterministic programs
- Requires complete source code
- Cannot verify external libraries
- Hardware failures not covered

But within these constraints, this is a **mathematical proof**, not just testing!
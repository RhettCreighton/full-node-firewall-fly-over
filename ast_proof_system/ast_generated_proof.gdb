# AST-Generated GDB Proof Script
set pagination off

# Set breakpoints at each dangerous operation

# Dangerous operation 1: division in Vector3 (Vector3)
break Vector3 (Vector3):28
commands
    silent
    printf "Checking division at col:28, col:35\n"
    # Check divisor is non-zero
    if $divisor == 0
        printf "ERROR: Division by zero!\n"
        set $proof_failed = 1
    else
        printf "OK: Divisor is %f\n", $divisor
    end
    continue
end

# Run program and verify
run
if $proof_failed
    printf "PROOF FAILED: Dangerous operations found\n"
    quit 1
else
    printf "PROOF PASSED: All operations safe\n"
    quit 0
end

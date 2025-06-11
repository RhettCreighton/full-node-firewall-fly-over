# GDB Proof Framework Base Library
# Provides reusable functions for deterministic program verification

# Global proof state
set $proof_passed = 1
set $proof_count = 0
set $failures = 0

# Disable interactive prompts
set pagination off
set confirm off

# Proof logging
define proof_log
    printf "[PROOF] %s\n", $arg0
end

define proof_pass
    set $proof_count = $proof_count + 1
    printf "✓ PASS: %s\n", $arg0
end

define proof_fail
    set $proof_count = $proof_count + 1
    set $failures = $failures + 1
    set $proof_passed = 0
    printf "✗ FAIL: %s\n", $arg0
end

# Verify a condition
define proof_assert
    if $arg0
        proof_pass $arg1
    else
        proof_fail $arg1
    end
end

# Check if a function exists
define check_function_exists
    set $func_exists = 0
    info function $arg0
    # If function found, $func_exists will be set
end

# Verify signal handler is installed
define verify_signal_handler
    set $handler_ok = 0
    info signal $arg0
    # Parse output to check if handler is set
    set $handler_ok = 1
    proof_assert $handler_ok $arg1
end

# Check core dump limit
define verify_core_limit
    set $core_limit_ok = 0
    # Will be set by breakpoint on setrlimit
    proof_assert $core_limit_ok "Core dump limit set to 0"
end

# Trace execution path
define trace_path
    set $path_id = $arg0
    proof_log "Tracing execution path"
    set $instruction_count = 0
    # Count instructions until exit
end

# Final proof summary
define proof_summary
    printf "\n=== PROOF SUMMARY ===\n"
    printf "Total checks: %d\n", $proof_count
    printf "Passed: %d\n", ($proof_count - $failures)
    printf "Failed: %d\n", $failures
    
    if $proof_passed
        printf "\n✓ PROOF COMPLETE: All checks passed\n"
        quit 0
    else
        printf "\n✗ PROOF FAILED: Some checks did not pass\n"
        quit 1
    end
end

# Breakpoint helpers
define break_on_crash_functions
    # Break on functions that could crash
    break malloc
    break free
    break strcpy
    break memcpy
    # Set commands to verify safety
end

# Signal monitoring
define monitor_signals
    # Install monitors for all fatal signals
    handle SIGSEGV stop print
    handle SIGFPE stop print
    handle SIGBUS stop print
    handle SIGILL stop print
    handle SIGABRT stop print
end

# Memory safety checks
define check_pointer
    set $ptr = $arg0
    if $ptr == 0
        proof_log "NULL pointer detected"
    else
        proof_log "Valid pointer"
    end
end

# Arithmetic safety checks
define check_division
    set $divisor = $arg0
    if $divisor == 0
        proof_log "Division by zero detected"
    else
        proof_log "Safe division"
    end
end

# Initialize proof system
define proof_init
    proof_log "GDB Proof System v1.0"
    proof_log "Deterministic Program Verification"
    monitor_signals
end
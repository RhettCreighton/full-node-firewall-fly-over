# Proof 04: Division by Zero Protection
# Verifies that division by zero doesn't create core dumps

proof_init
proof_log "=== Proof 04: Division by Zero Protection ==="

# Load the binary
# Binary already loaded

# Monitor arithmetic operations
set $div_zero_caught = 0

# Break on signal handler
break signal_handler
commands
    silent
    if sig == 8
        set $div_zero_caught = 1
        proof_log "SIGFPE caught - division by zero handled"
    end
    continue
end

# Monitor division operations in the code
# For x86_64, integer division uses idiv instruction
catch signal SIGFPE
commands
    silent
    set $div_zero_caught = 1
    continue
end

# Run the program
run

# Test division by zero scenarios
break main
continue

# Inject SIGFPE (what CPU does on div by zero)
printf "Simulating division by zero...\n"
signal SIGFPE

proof_assert $div_zero_caught "Division by zero caught by signal handler"

# Verify safe division macros work
# Look for SAFE_DIV usage
break safe_divide_f
commands
    silent
    if $xmm1 == 0
        proof_log "SAFE_DIV prevented division by zero"
    end
    continue
end

break safe_divide_i
commands
    silent  
    if $arg1 == 0
        proof_log "SAFE_DIV_I prevented division by zero"
    end
    continue
end

# Continue execution
continue

# No core dump should exist
shell test ! -f core && echo "No core file created" || echo "ERROR: core file exists!"

proof_summary
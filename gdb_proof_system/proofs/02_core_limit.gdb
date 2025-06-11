# Proof 02: Core Dump Limit Set to Zero
# Verifies that RLIMIT_CORE is set to 0 to prevent core dumps

proof_init
proof_log "=== Proof 02: Core Dump Limit Verification ==="

# Load the binary
# Binary already loaded

# Monitor setrlimit calls
set $core_limit_set = 0
set $core_limit_value = -1

break setrlimit
commands
    silent
    # Check if this is RLIMIT_CORE (value 4)
    if $arg0 == 4
        set $rlim = (struct rlimit *)$arg1
        set $core_limit_value = $rlim->rlim_cur
        set $core_limit_set = 1
        printf "RLIMIT_CORE set to: %d\n", $core_limit_value
    end
    continue
end

# Run the program
break main
run

# Continue until after initialization
break crash_protection_init
continue
finish

# Verify core limit was set
proof_assert $core_limit_set "setrlimit(RLIMIT_CORE) was called"
proof_assert ($core_limit_value == 0) "Core dump limit set to 0"

# Double-check with system
shell echo -n "System core limit: " && ulimit -c

# Test that we can't change it back
set $core_reenabled = 0
catch syscall setrlimit
commands
    silent
    if $rdi == 4
        set $new_limit = ((struct rlimit *)$rsi)->rlim_cur
        if $new_limit > 0
            set $core_reenabled = 1
            proof_fail "Core dumps re-enabled!"
        end
    end
    continue
end

# Continue execution for a bit
continue

# Final verification
proof_assert ($core_reenabled == 0) "Core dumps remain disabled"

proof_summary
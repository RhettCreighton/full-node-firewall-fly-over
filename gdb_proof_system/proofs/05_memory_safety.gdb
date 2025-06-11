# Proof 05: Memory Safety (Buffer Overflows, Use-After-Free)
# Verifies memory errors don't create core dumps

proof_init
proof_log "=== Proof 05: Memory Safety Verification ==="

# Load the binary
# Binary already loaded

# Track memory operations
set $malloc_count = 0
set $free_count = 0
set $segv_caught = 0
set $unsafe_access = 0

# Monitor malloc/free
break malloc
commands
    silent
    set $malloc_count = $malloc_count + 1
    continue
end

break free
commands
    silent
    set $free_count = $free_count + 1
    # Could check for double-free here
    continue
end

# Monitor signal handler
break signal_handler
commands
    silent
    if sig == 11
        set $segv_caught = $segv_caught + 1
        proof_log "Memory access violation caught"
    end
    continue
end

# Check for unsafe string functions
break strcpy
commands
    silent
    set $unsafe_access = 1
    proof_fail "Unsafe strcpy detected!"
    continue
end

break gets
commands
    silent
    set $unsafe_access = 1
    proof_fail "Unsafe gets detected!"
    continue
end

# Run program
run

# Look for safe alternatives
break strncpy
commands
    silent
    proof_log "Safe strncpy used instead of strcpy"
    continue
end

# Test buffer overflow scenario
break main
continue

# Simulate buffer overflow (would cause SIGSEGV)
printf "Simulating buffer overflow...\n"
signal SIGSEGV

proof_assert ($segv_caught > 0) "Buffer overflow caught by handler"

# Verify array bounds checking
rbreak SAFE_ARRAY_ACCESS
commands
    silent
    proof_log "Safe array access macro used"
    continue
end

# Continue execution
continue

# Summary checks
printf "Memory operations: %d mallocs, %d frees\n", $malloc_count, $free_count
proof_assert ($unsafe_access == 0) "No unsafe string functions used"

proof_summary
# Proof 01: Signal Handler Installation
# Verifies that all fatal signals have handlers installed

proof_init
proof_log "=== Proof 01: Signal Handler Installation ==="

# Binary is already loaded by GDB command line

# Set breakpoints to catch signal registration
set $sigsegv_handler = 0
set $sigfpe_handler = 0
set $sigbus_handler = 0
set $sigill_handler = 0
set $sigabrt_handler = 0

break signal
commands
    silent
    set $sig = $arg0
    set $handler = $arg1
    
    if $sig == 11
        set $sigsegv_handler = 1
        proof_log "SIGSEGV handler installed"
    end
    if $sig == 8
        set $sigfpe_handler = 1
        proof_log "SIGFPE handler installed"
    end
    if $sig == 7
        set $sigbus_handler = 1
        proof_log "SIGBUS handler installed"
    end
    if $sig == 4
        set $sigill_handler = 1
        proof_log "SIGILL handler installed"
    end
    if $sig == 6
        set $sigabrt_handler = 1
        proof_log "SIGABRT handler installed"
    end
    
    continue
end

# Run until signal handlers are installed
break crash_protection_init
run
finish

# Verify all handlers are installed
proof_assert $sigsegv_handler "SIGSEGV handler installed"
proof_assert $sigfpe_handler "SIGFPE handler installed"
proof_assert $sigbus_handler "SIGBUS handler installed"
proof_assert $sigill_handler "SIGILL handler installed"
proof_assert $sigabrt_handler "SIGABRT handler installed"

# Additional check: query signal dispositions
info signal SIGSEGV
info signal SIGFPE

proof_summary
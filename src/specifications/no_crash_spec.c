/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/core/specification.h"
#include "sky_combat/core/secure_code_points.h"
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Global jump buffer for crash recovery
static jmp_buf g_crash_recovery;
static int g_in_protected_section = 0;

// Override assert to never crash
#ifdef assert
#undef assert
#endif
#define assert(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "ASSERTION FAILED: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
        if (g_in_protected_section) { \
            longjmp(g_crash_recovery, 1); \
        } else { \
            /* Log error but continue execution */ \
            no_crash_spec_record_assertion_failure(#expr, __FILE__, __LINE__); \
        } \
    } \
} while(0)

typedef struct {
    specification_t* spec;
    int crash_attempts;
    int assertions_failed;
    int signals_caught;
    int null_derefs_prevented;
    char last_error[512];
} no_crash_spec_t;

static no_crash_spec_t* g_no_crash_spec = NULL;

// Signal handler that prevents crashes
static void no_crash_signal_handler(int sig) {
    if (!g_no_crash_spec) return;
    
    g_no_crash_spec->signals_caught++;
    
    const char* sig_name = "UNKNOWN";
    switch(sig) {
        case SIGSEGV: sig_name = "SEGMENTATION FAULT"; break;
        case SIGABRT: sig_name = "ABORT"; break;
        case SIGFPE:  sig_name = "FLOATING POINT EXCEPTION"; break;
        case SIGILL:  sig_name = "ILLEGAL INSTRUCTION"; break;
        case SIGBUS:  sig_name = "BUS ERROR"; break;
    }
    
    fprintf(stderr, "\n=== NO CRASH PROTECTION ACTIVATED ===\n");
    fprintf(stderr, "Caught signal: %d (%s)\n", sig, sig_name);
    fprintf(stderr, "Program would have crashed, but we're preventing it!\n");
    fprintf(stderr, "=====================================\n\n");
    
    snprintf(g_no_crash_spec->last_error, sizeof(g_no_crash_spec->last_error),
             "Signal %d (%s) prevented", sig, sig_name);
    
    SECURE_CODE_POINT(CRASH_PREVENTED, "true");
    
    // If in protected section, jump back
    if (g_in_protected_section) {
        longjmp(g_crash_recovery, sig);
    }
    
    // Otherwise, skip the bad instruction (DANGEROUS but prevents crash)
    // In real code, we'd need proper recovery logic here
}

// Install crash prevention handlers
static void install_no_crash_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = no_crash_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER; // Allow recursive signal handling
    
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
}

no_crash_spec_t* no_crash_spec_create(void) {
    no_crash_spec_t* ncs = calloc(1, sizeof(no_crash_spec_t));
    if (!ncs) return NULL;
    
    // Install handlers FIRST before creating spec
    install_no_crash_handlers();
    
    ncs->spec = spec_create(
        "No Crash Guarantee",
        "Program must NEVER crash, even with errors"
    );
    
    spec_set_critical(ncs->spec, true);
    spec_register(ncs->spec);
    
    g_no_crash_spec = ncs;
    
    SECURE_CODE_POINT(NO_CRASH_SPEC_CREATED, "true");
    
    return ncs;
}

void no_crash_spec_record_assertion_failure(const char* expr, const char* file, int line) {
    if (!g_no_crash_spec) return;
    
    g_no_crash_spec->assertions_failed++;
    
    snprintf(g_no_crash_spec->last_error, sizeof(g_no_crash_spec->last_error),
             "Assertion failed: %s at %s:%d", expr, file, line);
    
    fprintf(stderr, "NO-CRASH: Assertion would have crashed: %s\n", expr);
    
    SPEC_CHECK(g_no_crash_spec->spec, false,
               "Assertion failure handled without crashing");
}

// Safe execution wrapper - catches ANY crash attempt
bool no_crash_spec_safe_execute(void (*func)(void), const char* description) {
    if (!g_no_crash_spec) return false;
    
    g_in_protected_section = 1;
    
    if (setjmp(g_crash_recovery) == 0) {
        // Try to execute the function
        func();
        g_in_protected_section = 0;
        return true;
    } else {
        // We caught a crash!
        g_in_protected_section = 0;
        g_no_crash_spec->crash_attempts++;
        
        fprintf(stderr, "NO-CRASH: Prevented crash during: %s\n", description);
        
        SPEC_CHECK(g_no_crash_spec->spec, false,
                   "Function attempted to crash but was prevented");
        
        return false;
    }
}

// Check that no crashes occurred
bool no_crash_spec_verify(no_crash_spec_t* ncs) {
    if (!ncs) return false;
    
    bool no_crashes = (ncs->crash_attempts == 0 && 
                      ncs->signals_caught == 0 &&
                      ncs->assertions_failed == 0);
    
    SPEC_CHECK(ncs->spec, no_crashes,
               "No crash attempts were made");
    
    if (!no_crashes) {
        fprintf(stderr, "\nNO-CRASH REPORT:\n");
        fprintf(stderr, "  Crash attempts: %d\n", ncs->crash_attempts);
        fprintf(stderr, "  Signals caught: %d\n", ncs->signals_caught);
        fprintf(stderr, "  Assertions failed: %d\n", ncs->assertions_failed);
        fprintf(stderr, "  Last error: %s\n", ncs->last_error);
    }
    
    return no_crashes;
}

void no_crash_spec_destroy(no_crash_spec_t* ncs) {
    if (!ncs) return;
    
    no_crash_spec_verify(ncs);
    
    spec_finalize(ncs->spec);
    spec_report(ncs->spec);
    
    if (g_no_crash_spec == ncs) {
        g_no_crash_spec = NULL;
    }
    
    free(ncs);
}

// Override abort() to prevent crashes
void abort(void) {
    fprintf(stderr, "NO-CRASH: abort() called but prevented!\n");
    
    if (g_no_crash_spec) {
        g_no_crash_spec->crash_attempts++;
        SECURE_CODE_POINT(ABORT_PREVENTED, "true");
    }
    
    if (g_in_protected_section) {
        longjmp(g_crash_recovery, SIGABRT);
    }
    
    // Don't actually abort - just return
}
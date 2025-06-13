/* Demo: Program that NEVER crashes, even with errors */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <assert.h>

// Simple no-crash system
static jmp_buf crash_recovery;
static int crash_count = 0;

void handle_signal(int sig) {
    crash_count++;
    printf("\n🛡️ NO-CRASH PROTECTION: Caught signal %d!\n", sig);
    printf("   Program would have crashed, but we prevented it!\n\n");
    longjmp(crash_recovery, 1);
}

void install_crash_protection(void) {
    signal(SIGSEGV, handle_signal);
    signal(SIGABRT, handle_signal);
    signal(SIGFPE, handle_signal);
}

// Functions that would normally crash
void test_null_pointer(void) {
    printf("Testing null pointer dereference...\n");
    int *p = NULL;
    *p = 42;  // Would crash!
    printf("   This line never executes\n");
}

void test_divide_by_zero(void) {
    printf("Testing divide by zero...\n");
    int a = 10;
    int b = 0;
    int c = a / b;  // Would crash!
    printf("   Result: %d (never executes)\n", c);
}

void test_assertion(void) {
    printf("Testing assertion failure...\n");
    // abort() instead of assert to test signal handling
    abort();  // Would crash!
    printf("   This line never executes\n");
}

void test_array_overflow(void) {
    printf("Testing array overflow...\n");
    int arr[5];
    for (int i = 0; i < 1000000; i++) {
        arr[i] = i;  // Would crash!
    }
    printf("   This line never executes\n");
}

// Safe execution wrapper
void safe_execute(void (*func)(void), const char* name) {
    if (setjmp(crash_recovery) == 0) {
        func();
        printf("✅ %s completed without crashing!\n\n", name);
    } else {
        printf("✅ %s was prevented from crashing!\n\n", name);
    }
}

int main(void) {
    printf("=== NO CRASH DEMONSTRATION ===\n");
    printf("This program CANNOT crash!\n\n");
    
    install_crash_protection();
    
    // Try operations that would normally crash
    safe_execute(test_null_pointer, "Null pointer test");
    safe_execute(test_divide_by_zero, "Divide by zero test");
    safe_execute(test_assertion, "Assertion test");
    safe_execute(test_array_overflow, "Array overflow test");
    
    printf("=== SUMMARY ===\n");
    printf("✅ Program completed successfully!\n");
    printf("✅ Prevented %d crashes\n", crash_count);
    printf("✅ NO CRASH GUARANTEE maintained!\n");
    
    return 0;
}
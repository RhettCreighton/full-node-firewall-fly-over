/* Demo showing proper specification initialization order */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Minimal specification system to demonstrate the fix
typedef struct {
    int initialized;
} spec_system_t;

static spec_system_t g_spec_system = {0};

void spec_system_init(void) {
    printf("✅ Initializing specification system...\n");
    g_spec_system.initialized = 1;
}

void spec_register(const char* name) {
    // This assertion was causing the crash!
    assert(g_spec_system.initialized);
    printf("✅ Registered specification: %s\n", name);
}

// BAD: Creates spec without initializing system first
void bad_example(void) {
    printf("\n❌ BAD EXAMPLE - Will crash:\n");
    // spec_register("My Spec"); // Would crash with assertion failure!
    printf("   (Skipped to avoid crash)\n");
}

// GOOD: Initializes system before creating specs
void good_example(void) {
    printf("\n✅ GOOD EXAMPLE - Proper order:\n");
    
    // STEP 1: Initialize specification system FIRST
    spec_system_init();
    
    // STEP 2: Now safe to register specifications
    spec_register("Aircraft Controls");
    spec_register("Joystick Mapping");
    spec_register("Responsive Flight");
}

// Constructor attribute ensures this runs before main()
__attribute__((constructor))
static void auto_init(void) {
    printf("🔧 Auto-initialization running before main()...\n");
    // Could initialize spec system here for safety
}

int main(void) {
    printf("=== Specification Initialization Order Demo ===\n");
    
    bad_example();
    good_example();
    
    printf("\n✅ Specification system working correctly!\n");
    printf("✅ No crashes when proper init order is followed\n");
    
    return 0;
}
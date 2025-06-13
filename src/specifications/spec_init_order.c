/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/core/specification.h"
#include "sky_combat/core/secure_code_points.h"
#include <stdio.h>
#include <stdlib.h>

// This specification ensures the specification system is initialized before use
typedef struct {
    specification_t* spec;
    bool system_was_initialized;
    bool specs_registered_correctly;
    int initialization_order_violations;
} spec_init_order_t;

static spec_init_order_t* g_init_order_spec = NULL;

// This MUST be called before ANY specification creation
void spec_init_order_ensure_system_ready(void) {
    static bool init_checked = false;
    if (init_checked) return;
    init_checked = true;
    
    // CRITICAL: Initialize the specification system if not already done
    spec_system_init();
    
    SECURE_CODE_POINT(SPEC_SYSTEM_INITIALIZED, "true");
}

spec_init_order_t* spec_init_order_create(void) {
    // Ensure system is ready FIRST
    spec_init_order_ensure_system_ready();
    
    spec_init_order_t* sio = calloc(1, sizeof(spec_init_order_t));
    if (!sio) return NULL;
    
    sio->spec = spec_create(
        "Specification Init Order",
        "Specification system must be initialized before any specs are registered"
    );
    
    // This is critical - without proper init order, the program crashes
    spec_set_critical(sio->spec, true);
    spec_register(sio->spec);
    
    sio->system_was_initialized = true;
    
    // Store globally for other specs to check
    g_init_order_spec = sio;
    
    SECURE_CODE_POINT(SPEC_INIT_ORDER_CREATED, "true");
    
    return sio;
}

bool spec_init_order_check_before_create(void) {
    if (!g_init_order_spec) {
        fprintf(stderr, "ERROR: spec_init_order not created! Call spec_init_order_create() first!\n");
        return false;
    }
    
    SPEC_CHECK(g_init_order_spec->spec, 
               g_init_order_spec->system_was_initialized,
               "Specification system was initialized before use");
    
    return true;
}

void spec_init_order_report_violation(const char* spec_name) {
    if (!g_init_order_spec) return;
    
    g_init_order_spec->initialization_order_violations++;
    
    SPEC_CHECK(g_init_order_spec->spec, false,
               "Specification created before system initialization");
    
    fprintf(stderr, "INIT ORDER VIOLATION: %s created before spec_system_init()\n", spec_name);
}

void spec_init_order_destroy(spec_init_order_t* sio) {
    if (!sio) return;
    
    SPEC_CHECK(sio->spec, 
               sio->initialization_order_violations == 0,
               "No initialization order violations occurred");
    
    spec_finalize(sio->spec);
    spec_report(sio->spec);
    
    if (g_init_order_spec == sio) {
        g_init_order_spec = NULL;
    }
    
    free(sio);
}

// Auto-initialization helper for main programs
__attribute__((constructor))
static void spec_auto_init(void) {
    // This runs before main() to ensure spec system is ready
    spec_init_order_ensure_system_ready();
}
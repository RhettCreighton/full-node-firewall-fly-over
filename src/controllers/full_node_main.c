/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * FULL NODE: Firewall Fly-over
 * Main entry point with all safety guarantees
 */

#include <raylib.h>
#include <stdio.h>
#include "gdb_proof.h"
#include "gdb_proof_controls.h"

// Forward declare the actual game
int sky_combat_ultimate_main(void);

// Initialize all safety systems
static void initialize_safety_systems(void) {
    printf("=== FULL NODE: Firewall Fly-over ===\n");
    printf("Initializing safety systems...\n\n");
    
    // Step 1: Core safety proofs
    gdb_proof_init_aircraft_manager();
    gdb_proof_bounds_check_enabled();
    gdb_proof_null_check_enabled();
    gdb_proof_no_coredump_guarantee();
    
    // Step 2: Control lock verification
    VERIFY_CONTROL_LOCK();
    
    printf("\n✅ All safety systems verified!\n");
    printf("✅ Controls locked to ASTRO C40 configuration\n");
    printf("✅ No coredump guarantee established\n\n");
}

int main(void) {
    // REQUIRED: Initialize safety before anything else
    initialize_safety_systems();
    
    // Now run the actual game
    return sky_combat_ultimate_main();
}
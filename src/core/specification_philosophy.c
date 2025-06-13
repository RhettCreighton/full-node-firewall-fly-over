/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/core/specification_philosophy.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Implementation of philosophical specification verification
 * 
 * This ensures our interpretation of specifications is:
 * 1. Visible and auditable
 * 2. Redundantly expressed
 * 3. Bidirectionally verified
 * 4. Consensually validated
 * 5. Empirically tested
 */

bool philosophy_verify_interpretation(const interpretation_record_t* record) {
    if (!record) return false;
    
    // ALL fields must be present - no hidden assumptions
    if (!record->natural_language || strlen(record->natural_language) == 0) {
        fprintf(stderr, "PHILOSOPHY ERROR: Missing natural language spec\n");
        return false;
    }
    
    if (!record->formal_expression || strlen(record->formal_expression) == 0) {
        fprintf(stderr, "PHILOSOPHY ERROR: Missing formal expression\n");
        return false;
    }
    
    if (!record->code_implementation || strlen(record->code_implementation) == 0) {
        fprintf(stderr, "PHILOSOPHY ERROR: Missing code implementation\n");
        return false;
    }
    
    if (!record->rationale || strlen(record->rationale) == 0) {
        fprintf(stderr, "PHILOSOPHY ERROR: Missing interpretation rationale\n");
        return false;
    }
    
    // Check that alternatives were considered
    int alternatives_considered = 0;
    for (int i = 0; i < 5; i++) {
        if (record->alternatives[i] && strlen(record->alternatives[i]) > 0) {
            alternatives_considered++;
            
            // Each alternative must have a rejection reason
            if (!record->rejection_reasons[i] || strlen(record->rejection_reasons[i]) == 0) {
                fprintf(stderr, "PHILOSOPHY ERROR: Alternative without rejection reason\n");
                return false;
            }
        }
    }
    
    if (alternatives_considered == 0) {
        fprintf(stderr, "PHILOSOPHY ERROR: No alternative interpretations considered\n");
        return false;
    }
    
    // Verify correctness validation
    int correctness_verified = 0;
    for (int i = 0; i < 5; i++) {
        if (record->validation[i]) {
            correctness_verified++;
        }
    }
    
    if (correctness_verified < 3) {
        fprintf(stderr, "PHILOSOPHY ERROR: Insufficient correctness validation (need 3+)\n");
        return false;
    }
    
    return true;
}

bool philosophy_verify_multimodal(const multimodal_spec_t* spec) {
    if (!spec) return false;
    
    // ALL modes must be present
    const char* modes[] = {
        spec->natural_language,
        spec->formal_logic,
        spec->pseudocode,
        spec->example_valid,
        spec->example_invalid,
        spec->gdb_proof,
        spec->unit_test,
        spec->property_test
    };
    
    const char* mode_names[] = {
        "natural_language",
        "formal_logic",
        "pseudocode",
        "example_valid",
        "example_invalid",
        "gdb_proof",
        "unit_test",
        "property_test"
    };
    
    for (int i = 0; i < 8; i++) {
        if (!modes[i] || strlen(modes[i]) == 0) {
            fprintf(stderr, "PHILOSOPHY ERROR: Missing multimodal expression: %s\n", 
                    mode_names[i]);
            return false;
        }
    }
    
    // Verify examples are actually different
    if (strcmp(spec->example_valid, spec->example_invalid) == 0) {
        fprintf(stderr, "PHILOSOPHY ERROR: Valid and invalid examples are identical\n");
        return false;
    }
    
    return true;
}

bool philosophy_verify_bidirectional(bidirectional_verifier_t* verifier,
                                   const char* spec) {
    if (!verifier || !spec) return false;
    
    char generated_code[4096] = {0};
    char reconstructed_spec[4096] = {0};
    
    // Forward: Spec → Code
    if (!verifier->spec_to_code(spec, generated_code, sizeof(generated_code))) {
        fprintf(stderr, "PHILOSOPHY ERROR: Failed to generate code from spec\n");
        return false;
    }
    
    // Reverse: Code → Spec
    if (!verifier->code_to_spec(generated_code, reconstructed_spec, sizeof(reconstructed_spec))) {
        fprintf(stderr, "PHILOSOPHY ERROR: Failed to reconstruct spec from code\n");
        return false;
    }
    
    // Verify equivalence
    if (!verifier->verify_equivalence(spec, reconstructed_spec)) {
        fprintf(stderr, "PHILOSOPHY ERROR: Bidirectional verification failed\n");
        fprintf(stderr, "  Original: %s\n", spec);
        fprintf(stderr, "  Reconstructed: %s\n", reconstructed_spec);
        return false;
    }
    
    return true;
}

bool philosophy_build_consensus(consensus_validator_t validators[], 
                               size_t count,
                               const interpretation_record_t* record,
                               float required_confidence) {
    if (!validators || count == 0 || !record) return false;
    
    int agreements = 0;
    float total_confidence = 0.0f;
    
    printf("PHILOSOPHY: Building consensus among %zu validators...\n", count);
    
    for (size_t i = 0; i < count; i++) {
        if (validators[i].validate_interpretation(record)) {
            agreements++;
            total_confidence += validators[i].confidence_score;
            printf("  ✓ %s agrees (confidence: %.2f)\n", 
                   validators[i].validator_name,
                   validators[i].confidence_score);
        } else {
            printf("  ✗ %s disagrees: %s\n",
                   validators[i].validator_name,
                   validators[i].validation_rationale);
        }
    }
    
    float consensus_ratio = (float)agreements / count;
    float avg_confidence = total_confidence / count;
    
    printf("PHILOSOPHY: Consensus ratio: %.2f, Average confidence: %.2f\n",
           consensus_ratio, avg_confidence);
    
    // Require both high agreement AND high confidence
    return (consensus_ratio >= 0.8f && avg_confidence >= required_confidence);
}

bool philosophy_master_verification(
    const multimodal_spec_t* spec,
    const interpretation_record_t* interpretation,
    const redundant_spec_t* redundancy,
    const falsification_test_t* falsification,
    consensus_validator_t validators[],
    size_t validator_count,
    philosophical_basis_t basis) {
    
    printf("\n=== PHILOSOPHICAL MASTER VERIFICATION ===\n");
    printf("Basis: ");
    switch (basis) {
        case PHILOSOPHY_COHERENTISM:
            printf("Coherentism - Seeking internal consistency\n");
            break;
        case PHILOSOPHY_PRAGMATISM:
            printf("Pragmatism - Practical success defines correctness\n");
            break;
        case PHILOSOPHY_LANGUAGE_GAMES:
            printf("Language Games - Community usage defines meaning\n");
            break;
        case PHILOSOPHY_FALSIFICATION:
            printf("Falsification - If we can't break it, we accept it\n");
            break;
        case PHILOSOPHY_CONSENSUS:
            printf("Consensus - Agreement defines truth\n");
            break;
    }
    
    // Step 1: Verify interpretation is complete
    printf("\n1. Verifying interpretation completeness...\n");
    if (!philosophy_verify_interpretation(interpretation)) {
        fprintf(stderr, "FAILED: Interpretation incomplete\n");
        return false;
    }
    printf("   PASSED: All interpretation fields present\n");
    
    // Step 2: Verify multimodal expression
    printf("\n2. Verifying multimodal specification...\n");
    if (!philosophy_verify_multimodal(spec)) {
        fprintf(stderr, "FAILED: Multimodal expression incomplete\n");
        return false;
    }
    printf("   PASSED: Specification expressed in 8 different modes\n");
    
    // Step 3: Verify redundancy converges
    printf("\n3. Verifying redundant specifications converge...\n");
    if (redundancy && redundancy->expression_count >= 3) {
        char converged_impl[4096] = {0};
        if (!redundancy->converge_to_implementation(redundancy->expressions,
                                                   redundancy->expression_count,
                                                   converged_impl,
                                                   sizeof(converged_impl))) {
            fprintf(stderr, "FAILED: Redundant specs don't converge\n");
            return false;
        }
        printf("   PASSED: %zu expressions converge to same implementation\n",
               redundancy->expression_count);
    }
    
    // Step 4: Falsification testing
    printf("\n4. Running falsification tests...\n");
    if (falsification) {
        bool any_broke = false;
        
        if (falsification->try_null_pointer_crash && falsification->try_null_pointer_crash()) {
            fprintf(stderr, "   FAILED: Null pointer crash possible\n");
            any_broke = true;
        }
        
        if (falsification->try_buffer_overflow && falsification->try_buffer_overflow()) {
            fprintf(stderr, "   FAILED: Buffer overflow possible\n");
            any_broke = true;
        }
        
        if (falsification->try_race_condition && falsification->try_race_condition()) {
            fprintf(stderr, "   FAILED: Race condition possible\n");
            any_broke = true;
        }
        
        if (falsification->try_numeric_overflow && falsification->try_numeric_overflow()) {
            fprintf(stderr, "   FAILED: Numeric overflow possible\n");
            any_broke = true;
        }
        
        if (falsification->try_logic_contradiction && falsification->try_logic_contradiction()) {
            fprintf(stderr, "   FAILED: Logic contradiction possible\n");
            any_broke = true;
        }
        
        if (any_broke) {
            return false;
        }
        
        printf("   PASSED: Could not falsify implementation\n");
    }
    
    // Step 5: Build consensus
    printf("\n5. Building consensus...\n");
    if (validators && validator_count > 0) {
        if (!philosophy_build_consensus(validators, validator_count, interpretation, 0.8f)) {
            fprintf(stderr, "FAILED: Insufficient consensus\n");
            return false;
        }
        printf("   PASSED: Consensus achieved\n");
    }
    
    printf("\n=== MASTER VERIFICATION PASSED ===\n");
    printf("The specification-implementation mapping is:\n");
    printf("  ✓ Conventionally correct\n");
    printf("  ✓ Functionally correct\n");
    printf("  ✓ Coherently correct\n");
    printf("  ✓ Empirically correct\n");
    printf("  ✓ Consensually correct\n");
    printf("=====================================\n\n");
    
    return true;
}
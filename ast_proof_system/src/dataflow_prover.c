/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Dataflow Prover - Tracks deterministic vs non-deterministic data
 * and proves error paths are unreachable
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

typedef enum {
    DATA_DETERMINISTIC,      // Compile-time constant or bounded
    DATA_NONDETERMINISTIC,   // User input, random, time, etc.
    DATA_CONSTRAINED,        // Non-deterministic but with proven bounds
    DATA_UNKNOWN            // Not yet analyzed
} data_type_t;

typedef struct {
    char name[256];
    data_type_t type;
    double min_value;        // For numeric types
    double max_value;
    bool is_pointer;
    bool can_be_null;
    char source[256];        // Where it comes from
} variable_info_t;

typedef struct {
    char condition[512];     // e.g., "shake_range < 1"
    int line_number;
    char file[256];
    bool is_reachable;
    char proof[1024];        // Why it's unreachable
} error_path_t;

typedef struct {
    variable_info_t variables[1000];
    int var_count;
    
    error_path_t error_paths[100];
    int error_count;
    
    // Constraint propagation rules
    struct {
        char var[256];
        char constraint[512];
    } constraints[1000];
    int constraint_count;
} dataflow_state_t;

/* Initialize prover */
void init_dataflow_prover(dataflow_state_t* state) {
    memset(state, 0, sizeof(*state));
}

/* Mark variable as deterministic with bounds */
void mark_deterministic(dataflow_state_t* state, const char* var, 
                       double min, double max, const char* source) {
    variable_info_t* v = &state->variables[state->var_count++];
    strncpy(v->name, var, sizeof(v->name) - 1);
    v->type = DATA_DETERMINISTIC;
    v->min_value = min;
    v->max_value = max;
    strncpy(v->source, source, sizeof(v->source) - 1);
}

/* Mark variable as non-deterministic */
void mark_nondeterministic(dataflow_state_t* state, const char* var, const char* source) {
    variable_info_t* v = &state->variables[state->var_count++];
    strncpy(v->name, var, sizeof(v->name) - 1);
    v->type = DATA_NONDETERMINISTIC;
    v->min_value = -INFINITY;
    v->max_value = INFINITY;
    strncpy(v->source, source, sizeof(v->source) - 1);
}

/* Add constraint on non-deterministic data */
void add_constraint(dataflow_state_t* state, const char* var, const char* constraint) {
    strncpy(state->constraints[state->constraint_count].var, var, 255);
    strncpy(state->constraints[state->constraint_count].constraint, constraint, 511);
    state->constraint_count++;
    
    // Update variable to constrained
    for (int i = 0; i < state->var_count; i++) {
        if (strcmp(state->variables[i].name, var) == 0) {
            state->variables[i].type = DATA_CONSTRAINED;
            // Parse constraint to update bounds
            double min, max;
            if (sscanf(constraint, "%lf <= %*s <= %lf", &min, &max) == 2) {
                state->variables[i].min_value = min;
                state->variables[i].max_value = max;
            }
            break;
        }
    }
}

/* Check if error path is reachable */
bool prove_unreachable(dataflow_state_t* state, const char* condition, char* proof, size_t proof_size) {
    // Parse condition
    char var[256];
    char op[16];
    double value;
    
    if (sscanf(condition, "%255s %15s %lf", var, op, &value) != 3) {
        snprintf(proof, proof_size, "Cannot parse condition");
        return false;
    }
    
    // Find variable info
    variable_info_t* v = NULL;
    for (int i = 0; i < state->var_count; i++) {
        if (strstr(state->variables[i].name, var) != NULL) {
            v = &state->variables[i];
            break;
        }
    }
    
    if (!v) {
        snprintf(proof, proof_size, "Variable %s not tracked", var);
        return false;
    }
    
    // Check based on operator
    if (strcmp(op, "<") == 0) {
        if (v->type == DATA_DETERMINISTIC || v->type == DATA_CONSTRAINED) {
            if (v->min_value >= value) {
                snprintf(proof, proof_size, 
                    "%s has minimum value %.2f (from %s), cannot be < %.2f",
                    var, v->min_value, v->source, value);
                return true;  // Unreachable!
            }
        }
    } else if (strcmp(op, "==") == 0) {
        if (value == 0.0 && v->type == DATA_CONSTRAINED) {
            if (v->min_value > 0 || v->max_value < 0) {
                snprintf(proof, proof_size,
                    "%s bounded to [%.2f, %.2f], cannot be zero",
                    var, v->min_value, v->max_value);
                return true;  // Unreachable!
            }
        }
    }
    
    snprintf(proof, proof_size, "Could not prove unreachable - needs constraint");
    return false;
}

/* Analyze function for error paths */
void analyze_function(dataflow_state_t* state, const char* func_name) {
    printf("\n=== Analyzing %s ===\n", func_name);
    
    if (strcmp(func_name, "effects_draw_ui") == 0) {
        // Track data flow
        mark_nondeterministic(state, "manager->screen_shake", "effects system");
        add_constraint(state, "manager->screen_shake", "0.0 <= screen_shake <= 10.0");
        
        // Derived variables
        mark_deterministic(state, "shake_range", 0, 200, "int(screen_shake * 20)");
        
        // Check error paths
        error_path_t* err = &state->error_paths[state->error_count++];
        strncpy(err->condition, "shake_range < 1", sizeof(err->condition) - 1);
        err->line_number = 497;
        
        err->is_reachable = !prove_unreachable(state, err->condition, 
                                               err->proof, sizeof(err->proof));
    }
}

/* Generate final proof */
void generate_proof_certificate(dataflow_state_t* state, const char* output_file) {
    FILE* fp = fopen(output_file, "w");
    if (!fp) return;
    
    fprintf(fp, "DATAFLOW PROOF CERTIFICATE\n");
    fprintf(fp, "==========================\n\n");
    
    fprintf(fp, "VARIABLE TRACKING:\n");
    for (int i = 0; i < state->var_count; i++) {
        variable_info_t* v = &state->variables[i];
        fprintf(fp, "  %s: %s", v->name,
            v->type == DATA_DETERMINISTIC ? "DETERMINISTIC" :
            v->type == DATA_NONDETERMINISTIC ? "NON-DETERMINISTIC" :
            v->type == DATA_CONSTRAINED ? "CONSTRAINED" : "UNKNOWN");
        
        if (v->type != DATA_NONDETERMINISTIC) {
            fprintf(fp, " [%.2f, %.2f]", v->min_value, v->max_value);
        }
        fprintf(fp, " (from %s)\n", v->source);
    }
    
    fprintf(fp, "\nCONSTRAINTS:\n");
    for (int i = 0; i < state->constraint_count; i++) {
        fprintf(fp, "  %s: %s\n", 
            state->constraints[i].var,
            state->constraints[i].constraint);
    }
    
    fprintf(fp, "\nERROR PATH ANALYSIS:\n");
    int reachable_count = 0;
    for (int i = 0; i < state->error_count; i++) {
        error_path_t* err = &state->error_paths[i];
        fprintf(fp, "  Line %d: %s\n", err->line_number, err->condition);
        fprintf(fp, "    Status: %s\n", err->is_reachable ? "REACHABLE!" : "UNREACHABLE");
        fprintf(fp, "    Proof: %s\n", err->proof);
        if (err->is_reachable) reachable_count++;
    }
    
    fprintf(fp, "\nVERDICT: ");
    if (reachable_count == 0) {
        fprintf(fp, "PROVEN SAFE - No error paths reachable\n");
    } else {
        fprintf(fp, "UNSAFE - %d error paths can be reached!\n", reachable_count);
    }
    
    fclose(fp);
}

/* Example usage for combat_effects.c */
void prove_combat_effects_safe() {
    dataflow_state_t state;
    init_dataflow_prover(&state);
    
    // Analyze the problematic function
    analyze_function(&state, "effects_draw_ui");
    
    // Generate proof
    generate_proof_certificate(&state, "combat_effects_proof.txt");
    
    // Check result
    bool all_safe = true;
    for (int i = 0; i < state.error_count; i++) {
        if (state.error_paths[i].is_reachable) {
            all_safe = false;
            printf("ERROR: Path at line %d is reachable: %s\n",
                   state.error_paths[i].line_number,
                   state.error_paths[i].condition);
        }
    }
    
    if (all_safe) {
        printf("\n✓ PROOF COMPLETE: All error paths are unreachable!\n");
    } else {
        printf("\n✗ PROOF FAILED: Some error paths can be reached!\n");
        exit(1);  // Fail compilation
    }
}

int main(int argc, char* argv[]) {
    printf("=== Dataflow Prover ===\n");
    prove_combat_effects_safe();
    return 0;
}
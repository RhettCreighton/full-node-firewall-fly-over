/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "constraint_verifier.h"

/* Constraint patterns to look for */
typedef struct {
    const char* pattern;
    constraint_type_t type;
    const char* description;
} constraint_pattern_t;

static const constraint_pattern_t constraint_patterns[] = {
    /* Range checks */
    {">=", CONSTRAINT_RANGE, "Lower bound check"},
    {"<=", CONSTRAINT_RANGE, "Upper bound check"},
    {">", CONSTRAINT_RANGE, "Lower bound check"},
    {"<", CONSTRAINT_RANGE, "Upper bound check"},
    
    /* Equality/inequality */
    {"!=", CONSTRAINT_INEQUALITY, "Not equal check"},
    {"==", CONSTRAINT_EQUALITY, "Equality check"},
    
    /* NULL checks */
    {"!= NULL", CONSTRAINT_NULL_CHECK, "NULL pointer check"},
    {"NULL !=", CONSTRAINT_NULL_CHECK, "NULL pointer check"},
    {"!", CONSTRAINT_NULL_CHECK, "Negation check"},
    
    /* Function calls */
    {"clamp", CONSTRAINT_CLAMPED, "Value clamping"},
    {"fmax", CONSTRAINT_CLAMPED, "Maximum bound"},
    {"fmin", CONSTRAINT_CLAMPED, "Minimum bound"},
    {"validate_", CONSTRAINT_VALIDATED, "Custom validation"},
    {"is_valid_", CONSTRAINT_VALIDATED, "Validity check"},
    
    {NULL, CONSTRAINT_UNCONSTRAINED, NULL}
};

/* Initialize constraint verifier */
void init_constraint_verifier(constraint_verifier_t* verifier) {
    memset(verifier, 0, sizeof(*verifier));
}

/* Extract variable name from expression */
static void extract_variable(const char* expr, char* var_name, size_t max_len) {
    const char* p = expr;
    char* out = var_name;
    size_t len = 0;
    
    /* Skip whitespace */
    while (*p && isspace(*p)) p++;
    
    /* Extract identifier */
    while (*p && (isalnum(*p) || *p == '_') && len < max_len - 1) {
        *out++ = *p++;
        len++;
    }
    
    *out = '\0';
}

/* Parse constraint from AST node */
static constraint_t* parse_constraint(const ast_node_t* node) {
    constraint_t* constraint = malloc(sizeof(constraint_t));
    if (!constraint) return NULL;
    
    memset(constraint, 0, sizeof(*constraint));
    strncpy(constraint->location, node->location, sizeof(constraint->location) - 1);
    
    /* Check for constraint patterns */
    for (int i = 0; constraint_patterns[i].pattern != NULL; i++) {
        if (strstr(node->content, constraint_patterns[i].pattern)) {
            constraint->type = constraint_patterns[i].type;
            strncpy(constraint->expression, node->content, 
                   sizeof(constraint->expression) - 1);
            
            /* Extract bounds for range constraints */
            if (constraint->type == CONSTRAINT_RANGE) {
                /* Simple number extraction */
                char* num_start = strpbrk(node->content, "0123456789-");
                if (num_start) {
                    double value = strtod(num_start, NULL);
                    if (strstr(node->content, ">=") || strstr(node->content, ">")) {
                        constraint->min_value = value;
                        constraint->has_min = true;
                    } else {
                        constraint->max_value = value;
                        constraint->has_max = true;
                    }
                }
            }
            
            constraint->is_verified = true;
            return constraint;
        }
    }
    
    free(constraint);
    return NULL;
}

/* Find constraints for a variable */
int find_variable_constraints(constraint_verifier_t* verifier,
                            const char* var_name,
                            const ast_node_t* nodes,
                            int node_count,
                            int start_index) {
    if (verifier->constraint_count >= MAX_CONSTRAINTS) {
        return -1;
    }
    
    input_constraint_t* input = &verifier->constraints[verifier->constraint_count];
    strncpy(input->variable_name, var_name, sizeof(input->variable_name) - 1);
    input->constraint_count = 0;
    
    /* Look for constraints after the variable usage */
    for (int i = start_index; i < node_count && i < start_index + 50; i++) {
        if (nodes[i].type == 6 || nodes[i].type == 1) { /* AST_IF_STMT || AST_BINARY_OP */
            /* Check if this node constrains our variable */
            if (strstr(nodes[i].content, var_name)) {
                constraint_t* c = parse_constraint(&nodes[i]);
                if (c && input->constraint_count < MAX_GUARDS) {
                    memcpy(&input->constraints[input->constraint_count], c, 
                          sizeof(constraint_t));
                    input->constraint_count++;
                    free(c);
                }
            }
        }
    }
    
    /* Determine safety status */
    input->safety_status = SAFETY_UNSAFE;
    
    for (int i = 0; i < input->constraint_count; i++) {
        constraint_t* c = &input->constraints[i];
        
        if (c->type == CONSTRAINT_NULL_CHECK ||
            c->type == CONSTRAINT_CLAMPED ||
            c->type == CONSTRAINT_VALIDATED) {
            input->safety_status = SAFETY_SAFE;
            break;
        } else if (c->type == CONSTRAINT_RANGE && c->has_min && c->has_max) {
            input->safety_status = SAFETY_SAFE;
            break;
        } else if (c->type == CONSTRAINT_INEQUALITY) {
            input->safety_status = SAFETY_PARTIAL;
        }
    }
    
    verifier->constraint_count++;
    return 0;
}

/* Verify constraints for an operation */
bool verify_operation_constraints(const constraint_verifier_t* verifier,
                                const dangerous_op_t* op,
                                constraint_violation_t* violation) {
    /* Find constraints for this operation */
    const input_constraint_t* relevant_constraints = NULL;
    
    for (int i = 0; i < verifier->constraint_count; i++) {
        if (strcmp(verifier->constraints[i].function, op->function) == 0) {
            relevant_constraints = &verifier->constraints[i];
            break;
        }
    }
    
    /* Check based on operation type */
    bool has_required_constraint = false;
    
    if (op->type == OP_DIVISION || op->type == OP_MODULO) {
        /* Need non-zero check */
        if (relevant_constraints) {
            for (int i = 0; i < relevant_constraints->constraint_count; i++) {
                const constraint_t* c = &relevant_constraints->constraints[i];
                if (c->type == CONSTRAINT_INEQUALITY ||
                    (c->type == CONSTRAINT_RANGE && c->has_min && c->min_value > 0)) {
                    has_required_constraint = true;
                    break;
                }
            }
        }
        
        if (!has_required_constraint && violation) {
            violation->operation_type = op->type;
            strncpy(violation->location, op->location, sizeof(violation->location) - 1);
            strncpy(violation->missing_constraint, "divisor != 0", 
                   sizeof(violation->missing_constraint) - 1);
            violation->severity = SEVERITY_CRITICAL;
            strncpy(violation->fix_suggestion,
                   "Add check: if (divisor != 0) { result = a / divisor; }",
                   sizeof(violation->fix_suggestion) - 1);
        }
    } else if (op->type == OP_DEREF) {
        /* Need NULL check */
        if (relevant_constraints) {
            for (int i = 0; i < relevant_constraints->constraint_count; i++) {
                if (relevant_constraints->constraints[i].type == CONSTRAINT_NULL_CHECK) {
                    has_required_constraint = true;
                    break;
                }
            }
        }
        
        if (!has_required_constraint && violation) {
            violation->operation_type = op->type;
            strncpy(violation->location, op->location, sizeof(violation->location) - 1);
            strncpy(violation->missing_constraint, "ptr != NULL",
                   sizeof(violation->missing_constraint) - 1);
            violation->severity = SEVERITY_CRITICAL;
            strncpy(violation->fix_suggestion,
                   "Add check: if (ptr != NULL) { use(ptr); }",
                   sizeof(violation->fix_suggestion) - 1);
        }
    } else if (op->type == OP_ARRAY_ACCESS) {
        /* Need bounds check */
        if (relevant_constraints) {
            for (int i = 0; i < relevant_constraints->constraint_count; i++) {
                const constraint_t* c = &relevant_constraints->constraints[i];
                if (c->type == CONSTRAINT_RANGE || c->type == CONSTRAINT_CLAMPED) {
                    has_required_constraint = true;
                    break;
                }
            }
        }
        
        if (!has_required_constraint && violation) {
            violation->operation_type = op->type;
            strncpy(violation->location, op->location, sizeof(violation->location) - 1);
            strncpy(violation->missing_constraint, "0 <= index < size",
                   sizeof(violation->missing_constraint) - 1);
            violation->severity = SEVERITY_CRITICAL;
            strncpy(violation->fix_suggestion,
                   "Add check: if (index >= 0 && index < size) { array[index]; }",
                   sizeof(violation->fix_suggestion) - 1);
        }
    }
    
    return has_required_constraint;
}

/* Calculate constraint coverage */
void calculate_constraint_coverage(constraint_verifier_t* verifier) {
    int total_inputs = verifier->constraint_count;
    int safe_inputs = 0;
    int partial_inputs = 0;
    
    for (int i = 0; i < total_inputs; i++) {
        if (verifier->constraints[i].safety_status == SAFETY_SAFE) {
            safe_inputs++;
        } else if (verifier->constraints[i].safety_status == SAFETY_PARTIAL) {
            partial_inputs++;
        }
    }
    
    if (total_inputs > 0) {
        verifier->constraint_coverage = 
            (double)(safe_inputs + 0.5 * partial_inputs) / total_inputs;
    } else {
        verifier->constraint_coverage = 1.0;
    }
    
    /* Calculate safety score */
    double base_score = verifier->constraint_coverage * 100.0;
    double penalty = verifier->violation_count * 10.0;
    
    for (int i = 0; i < verifier->violation_count; i++) {
        if (verifier->violations[i].severity == SEVERITY_CRITICAL) {
            penalty += 10.0;
        }
    }
    
    verifier->safety_score = fmax(0.0, base_score - penalty);
}

/* Print constraint verification report */
void print_constraint_report(const constraint_verifier_t* verifier) {
    printf("=== Constraint Verification Report ===\n");
    printf("Total inputs analyzed: %d\n", verifier->constraint_count);
    printf("Constraint coverage: %.1f%%\n", verifier->constraint_coverage * 100);
    printf("Safety score: %.1f/100\n\n", verifier->safety_score);
    
    /* Show unconstrained inputs */
    int unsafe_count = 0;
    for (int i = 0; i < verifier->constraint_count; i++) {
        if (verifier->constraints[i].safety_status == SAFETY_UNSAFE) {
            unsafe_count++;
        }
    }
    
    if (unsafe_count > 0) {
        printf("Unconstrained inputs: %d\n", unsafe_count);
        for (int i = 0; i < verifier->constraint_count; i++) {
            const input_constraint_t* input = &verifier->constraints[i];
            if (input->safety_status == SAFETY_UNSAFE) {
                printf("  %s in %s - NO CONSTRAINTS\n", 
                      input->variable_name, input->function);
            }
        }
        printf("\n");
    }
    
    /* Show violations */
    if (verifier->violation_count > 0) {
        printf("Constraint violations: %d\n", verifier->violation_count);
        for (int i = 0; i < verifier->violation_count; i++) {
            const constraint_violation_t* v = &verifier->violations[i];
            printf("  %s at %s\n", v->missing_constraint, v->location);
            printf("    Fix: %s\n", v->fix_suggestion);
        }
    } else {
        printf("✓ All operations properly constrained!\n");
    }
}

/* Generate constraint proof */
int generate_constraint_proof(const constraint_verifier_t* verifier,
                            const char* output_file) {
    FILE* fp = fopen(output_file, "w");
    if (!fp) return -1;
    
    fprintf(fp, "# Runtime Constraint Verification\n");
    fprintf(fp, "# Verify all non-deterministic inputs are bounded\n\n");
    
    fprintf(fp, "define verify_constraints\n");
    fprintf(fp, "  set $violations = 0\n\n");
    
    /* Generate checks for each constrained input */
    for (int i = 0; i < verifier->constraint_count; i++) {
        const input_constraint_t* input = &verifier->constraints[i];
        
        fprintf(fp, "  # Verify %s in %s\n", input->variable_name, input->function);
        
        for (int j = 0; j < input->constraint_count; j++) {
            const constraint_t* c = &input->constraints[j];
            
            if (c->type == CONSTRAINT_RANGE) {
                if (c->has_min) {
                    fprintf(fp, "  # Check: %s >= %.2f\n", 
                           input->variable_name, c->min_value);
                }
                if (c->has_max) {
                    fprintf(fp, "  # Check: %s <= %.2f\n",
                           input->variable_name, c->max_value);
                }
            } else if (c->type == CONSTRAINT_NULL_CHECK) {
                fprintf(fp, "  # Check: %s != NULL\n", input->variable_name);
            }
        }
        
        fprintf(fp, "\n");
    }
    
    fprintf(fp, "  if $violations == 0\n");
    fprintf(fp, "    printf \"✓ All constraints verified\\n\"\n");
    fprintf(fp, "  else\n");
    fprintf(fp, "    printf \"✗ %%d constraint violations\\n\", $violations\n");
    fprintf(fp, "  end\n");
    fprintf(fp, "end\n\n");
    fprintf(fp, "verify_constraints\n");
    
    fclose(fp);
    return 0;
}
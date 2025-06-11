/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CONSTRAINT_VERIFIER_H
#define CONSTRAINT_VERIFIER_H

#include <stdbool.h>
#include "ast_analyzer.h"

/* Maximum limits */
#define MAX_CONSTRAINTS 1000
#define MAX_VIOLATIONS 100

/* Constraint types */
typedef enum {
    CONSTRAINT_RANGE = 0,
    CONSTRAINT_EQUALITY,
    CONSTRAINT_INEQUALITY,
    CONSTRAINT_NULL_CHECK,
    CONSTRAINT_ARRAY_BOUNDS,
    CONSTRAINT_CLAMPED,
    CONSTRAINT_VALIDATED,
    CONSTRAINT_UNCONSTRAINED
} constraint_type_t;

/* Safety status */
typedef enum {
    SAFETY_SAFE = 0,
    SAFETY_PARTIAL,
    SAFETY_UNSAFE
} safety_status_t;

/* Severity levels */
typedef enum {
    SEVERITY_LOW = 0,
    SEVERITY_MEDIUM,
    SEVERITY_HIGH,
    SEVERITY_CRITICAL
} severity_t;

/* Constraint information */
typedef struct {
    constraint_type_t type;
    char expression[512];
    double min_value;
    double max_value;
    bool has_min;
    bool has_max;
    char location[MAX_LOCATION_LEN];
    bool is_verified;
} constraint_t;

/* Input constraint */
typedef struct {
    char variable_name[128];
    char input_source[128];
    char location[MAX_LOCATION_LEN];
    char function[MAX_FUNCTION_LEN];
    constraint_t constraints[MAX_GUARDS];
    int constraint_count;
    safety_status_t safety_status;
} input_constraint_t;

/* Constraint violation */
typedef struct {
    int operation_type;
    char location[MAX_LOCATION_LEN];
    char variable[128];
    char missing_constraint[256];
    severity_t severity;
    char fix_suggestion[512];
} constraint_violation_t;

/* Constraint verifier state */
typedef struct {
    input_constraint_t constraints[MAX_CONSTRAINTS];
    int constraint_count;
    
    constraint_violation_t violations[MAX_VIOLATIONS];
    int violation_count;
    
    double constraint_coverage;
    double safety_score;
} constraint_verifier_t;

/* Functions */
void init_constraint_verifier(constraint_verifier_t* verifier);
int find_variable_constraints(constraint_verifier_t* verifier,
                            const char* var_name,
                            const ast_node_t* nodes,
                            int node_count,
                            int start_index);
bool verify_operation_constraints(const constraint_verifier_t* verifier,
                                const dangerous_op_t* op,
                                constraint_violation_t* violation);
void calculate_constraint_coverage(constraint_verifier_t* verifier);
void print_constraint_report(const constraint_verifier_t* verifier);
int generate_constraint_proof(const constraint_verifier_t* verifier,
                            const char* output_file);

#endif /* CONSTRAINT_VERIFIER_H */
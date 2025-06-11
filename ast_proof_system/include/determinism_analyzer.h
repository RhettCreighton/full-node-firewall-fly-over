/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DETERMINISM_ANALYZER_H
#define DETERMINISM_ANALYZER_H

#include <stdbool.h>
#include "ast_analyzer.h"

/* Maximum limits */
#define MAX_FUNCTIONS 1000
#define MAX_CALLS_PER_FUNC 100

/* Determinism levels */
typedef enum {
    DETERMINISM_DETERMINISTIC = 0,
    DETERMINISM_NON_DETERMINISTIC,
    DETERMINISM_CONDITIONALLY_DETERMINISTIC
} determinism_t;

/* Input sources (bitmask) */
typedef enum {
    INPUT_DETERMINISTIC = 0,
    INPUT_USER = (1 << 0),
    INPUT_TIME = (1 << 1),
    INPUT_RANDOM = (1 << 2),
    INPUT_SYSTEM = (1 << 3),
    INPUT_NETWORK = (1 << 4),
    INPUT_HARDWARE = (1 << 5)
} input_source_t;

/* Function information */
typedef struct {
    char name[MAX_FUNCTION_LEN];
    determinism_t determinism;
    input_source_t input_sources;
    char calls[MAX_CALLS_PER_FUNC][MAX_FUNCTION_LEN];
    int call_count;
    int nondet_call_count;
} function_info_t;

/* Optimization strategy */
typedef enum {
    OPT_NONE = 0,
    OPT_CACHE,
    OPT_ELIMINATE,
    OPT_PRECOMPUTE
} optimization_strategy_t;

/* Operation optimization info */
typedef struct {
    int operation_id;
    optimization_strategy_t strategy;
    bool can_cache;
    bool can_eliminate;
    double speedup_factor;
} operation_optimization_t;

/* Optimization statistics */
typedef struct {
    int total_operations;
    int cacheable_operations;
    int eliminable_operations;
    int precomputable_operations;
    double estimated_speedup;
} optimization_stats_t;

/* Determinism analyzer state */
typedef struct {
    function_info_t functions[MAX_FUNCTIONS];
    int function_count;
    int deterministic_count;
    int non_deterministic_count;
    
    operation_optimization_t operations[MAX_DANGEROUS_OPS];
    int operation_count;
    int total_operations;
    
    optimization_stats_t opt_stats;
} determinism_analyzer_t;

/* Functions */
void init_determinism_analyzer(determinism_analyzer_t* analyzer);
int analyze_function_determinism(determinism_analyzer_t* analyzer,
                                const char* func_name,
                                const ast_node_t* nodes,
                                int node_count);
void propagate_determinism(determinism_analyzer_t* analyzer);
void analyze_operation_optimization(determinism_analyzer_t* analyzer,
                                  const dangerous_op_t* op,
                                  operation_optimization_t* opt);
void calculate_optimization_stats(determinism_analyzer_t* analyzer);
void print_determinism_analysis(const determinism_analyzer_t* analyzer);
int generate_optimized_verification(const determinism_analyzer_t* analyzer,
                                   const char* output_file);

#endif /* DETERMINISM_ANALYZER_H */
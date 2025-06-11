/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "determinism_analyzer.h"

/* Non-deterministic function database */
typedef struct {
    const char* name;
    input_source_t source;
    const char* description;
} nondet_function_t;

static const nondet_function_t nondet_functions[] = {
    /* User input */
    {"scanf", INPUT_USER, "User input"},
    {"fscanf", INPUT_USER, "File input"},
    {"getchar", INPUT_USER, "Character input"},
    {"gets", INPUT_USER, "String input"},
    {"fgets", INPUT_USER, "Line input"},
    {"read", INPUT_USER, "Low-level input"},
    
    /* Time */
    {"time", INPUT_TIME, "Current time"},
    {"clock", INPUT_TIME, "Processor time"},
    {"gettimeofday", INPUT_TIME, "High-res time"},
    
    /* Random */
    {"rand", INPUT_RANDOM, "Random number"},
    {"random", INPUT_RANDOM, "Random number"},
    {"drand48", INPUT_RANDOM, "Random double"},
    
    /* System */
    {"getenv", INPUT_SYSTEM, "Environment variable"},
    {"getpid", INPUT_SYSTEM, "Process ID"},
    {"getcwd", INPUT_SYSTEM, "Current directory"},
    
    /* Network */
    {"recv", INPUT_NETWORK, "Network receive"},
    {"recvfrom", INPUT_NETWORK, "UDP receive"},
    {"accept", INPUT_NETWORK, "Accept connection"},
    
    {NULL, INPUT_DETERMINISTIC, NULL}
};

/* Initialize determinism analyzer */
void init_determinism_analyzer(determinism_analyzer_t* analyzer) {
    memset(analyzer, 0, sizeof(*analyzer));
}

/* Check if function is non-deterministic */
static input_source_t check_function_determinism(const char* func_name) {
    for (int i = 0; nondet_functions[i].name != NULL; i++) {
        if (strcmp(func_name, nondet_functions[i].name) == 0) {
            return nondet_functions[i].source;
        }
    }
    return INPUT_DETERMINISTIC;
}

/* Analyze function for determinism */
int analyze_function_determinism(determinism_analyzer_t* analyzer,
                                const char* func_name,
                                const ast_node_t* nodes,
                                int node_count) {
    if (analyzer->function_count >= MAX_FUNCTIONS) {
        return -1;
    }
    
    function_info_t* func = &analyzer->functions[analyzer->function_count];
    strncpy(func->name, func_name, sizeof(func->name) - 1);
    func->determinism = DETERMINISM_DETERMINISTIC;
    func->input_sources = INPUT_DETERMINISTIC;
    
    /* Scan nodes for non-deterministic calls */
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].type == 3) { /* AST_CALL_EXPR */
            /* Extract called function name */
            char called_func[128] = {0};
            const char* start = strstr(nodes[i].content, "'");
            if (start) {
                const char* end = strchr(start + 1, '\'');
                if (end) {
                    size_t len = end - start - 1;
                    if (len < sizeof(called_func) - 1) {
                        strncpy(called_func, start + 1, len);
                    }
                }
            }
            
            /* Check if non-deterministic */
            input_source_t source = check_function_determinism(called_func);
            if (source != INPUT_DETERMINISTIC) {
                func->determinism = DETERMINISM_NON_DETERMINISTIC;
                func->input_sources |= source;
                func->nondet_call_count++;
            }
        }
    }
    
    analyzer->function_count++;
    
    /* Update statistics */
    if (func->determinism == DETERMINISM_DETERMINISTIC) {
        analyzer->deterministic_count++;
    } else {
        analyzer->non_deterministic_count++;
    }
    
    return 0;
}

/* Propagate determinism through call graph */
void propagate_determinism(determinism_analyzer_t* analyzer) {
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        for (int i = 0; i < analyzer->function_count; i++) {
            function_info_t* caller = &analyzer->functions[i];
            
            if (caller->determinism == DETERMINISM_DETERMINISTIC) {
                /* Check if any called function is non-deterministic */
                for (int j = 0; j < caller->call_count; j++) {
                    const char* callee_name = caller->calls[j];
                    
                    /* Find callee */
                    for (int k = 0; k < analyzer->function_count; k++) {
                        if (strcmp(analyzer->functions[k].name, callee_name) == 0) {
                            if (analyzer->functions[k].determinism == DETERMINISM_NON_DETERMINISTIC) {
                                /* Propagate non-determinism */
                                caller->determinism = DETERMINISM_NON_DETERMINISTIC;
                                caller->input_sources |= analyzer->functions[k].input_sources;
                                analyzer->deterministic_count--;
                                analyzer->non_deterministic_count++;
                                changed = true;
                                break;
                            }
                        }
                    }
                    
                    if (changed) break;
                }
            }
        }
    }
}

/* Analyze operations for optimization potential */
void analyze_operation_optimization(determinism_analyzer_t* analyzer,
                                  const dangerous_op_t* op,
                                  operation_optimization_t* opt) {
    /* Find function containing operation */
    function_info_t* func = NULL;
    for (int i = 0; i < analyzer->function_count; i++) {
        if (strcmp(analyzer->functions[i].name, op->function) == 0) {
            func = &analyzer->functions[i];
            break;
        }
    }
    
    if (!func) {
        opt->strategy = OPT_NONE;
        return;
    }
    
    /* Determine optimization based on determinism */
    if (func->determinism == DETERMINISM_DETERMINISTIC) {
        if (op->is_safe) {
            opt->strategy = OPT_ELIMINATE;  /* Can remove after first proof */
            opt->speedup_factor = 10.0;
        } else {
            opt->strategy = OPT_CACHE;  /* Cache the proof */
            opt->speedup_factor = 5.0;
        }
        opt->can_cache = true;
        opt->can_eliminate = op->is_safe;
    } else {
        /* Non-deterministic operations */
        if (op->is_safe && (func->input_sources & INPUT_USER) == 0) {
            /* Bounded non-determinism (e.g., time-based) */
            opt->strategy = OPT_CACHE;
            opt->speedup_factor = 2.0;
            opt->can_cache = true;
        } else {
            /* Unbounded non-determinism */
            opt->strategy = OPT_NONE;
            opt->speedup_factor = 1.0;
        }
        opt->can_eliminate = false;
    }
}

/* Calculate overall optimization potential */
void calculate_optimization_stats(determinism_analyzer_t* analyzer) {
    optimization_stats_t* stats = &analyzer->opt_stats;
    
    stats->total_operations = analyzer->total_operations;
    
    double total_original_cost = 0.0;
    double total_optimized_cost = 0.0;
    
    for (int i = 0; i < analyzer->operation_count; i++) {
        const operation_optimization_t* opt = &analyzer->operations[i];
        
        /* Assume base cost of 1.0 per operation */
        total_original_cost += 1.0;
        
        switch (opt->strategy) {
        case OPT_ELIMINATE:
            stats->eliminable_operations++;
            total_optimized_cost += 0.0;  /* No cost after elimination */
            break;
            
        case OPT_CACHE:
            stats->cacheable_operations++;
            total_optimized_cost += 0.2;  /* Cache lookup cost */
            break;
            
        case OPT_PRECOMPUTE:
            stats->precomputable_operations++;
            total_optimized_cost += 0.1;  /* Precomputed value access */
            break;
            
        default:
            total_optimized_cost += 1.0;  /* Full cost */
            break;
        }
    }
    
    if (total_original_cost > 0) {
        stats->estimated_speedup = total_original_cost / total_optimized_cost;
    } else {
        stats->estimated_speedup = 1.0;
    }
}

/* Print determinism analysis */
void print_determinism_analysis(const determinism_analyzer_t* analyzer) {
    printf("=== Determinism Analysis ===\n");
    printf("Functions analyzed: %d\n", analyzer->function_count);
    printf("  Deterministic: %d\n", analyzer->deterministic_count);
    printf("  Non-deterministic: %d\n", analyzer->non_deterministic_count);
    printf("\n");
    
    /* Show non-deterministic functions */
    if (analyzer->non_deterministic_count > 0) {
        printf("Non-deterministic functions:\n");
        for (int i = 0; i < analyzer->function_count; i++) {
            const function_info_t* func = &analyzer->functions[i];
            if (func->determinism == DETERMINISM_NON_DETERMINISTIC) {
                printf("  %s: ", func->name);
                
                /* Show input sources */
                if (func->input_sources & INPUT_USER) printf("USER ");
                if (func->input_sources & INPUT_TIME) printf("TIME ");
                if (func->input_sources & INPUT_RANDOM) printf("RANDOM ");
                if (func->input_sources & INPUT_SYSTEM) printf("SYSTEM ");
                if (func->input_sources & INPUT_NETWORK) printf("NETWORK ");
                
                printf("\n");
            }
        }
    }
    
    /* Show optimization potential */
    const optimization_stats_t* stats = &analyzer->opt_stats;
    printf("\nOptimization potential:\n");
    printf("  Eliminable operations: %d\n", stats->eliminable_operations);
    printf("  Cacheable operations: %d\n", stats->cacheable_operations);
    printf("  Estimated speedup: %.1fx\n", stats->estimated_speedup);
}

/* Generate optimized verification code */
int generate_optimized_verification(const determinism_analyzer_t* analyzer,
                                   const char* output_file) {
    FILE* fp = fopen(output_file, "w");
    if (!fp) return -1;
    
    fprintf(fp, "/* SPDX-FileCopyrightText: 2025 Rhett Creighton\n");
    fprintf(fp, " * SPDX-License-Identifier: Apache-2.0\n");
    fprintf(fp, " */\n\n");
    
    fprintf(fp, "/* Optimized verification based on determinism analysis */\n\n");
    
    fprintf(fp, "#include <stdbool.h>\n");
    fprintf(fp, "#include <stdint.h>\n\n");
    
    /* Generate cache structure */
    fprintf(fp, "/* Proof cache for deterministic operations */\n");
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "    uint32_t operation_id;\n");
    fprintf(fp, "    bool verified;\n");
    fprintf(fp, "} proof_cache_entry_t;\n\n");
    
    fprintf(fp, "#define PROOF_CACHE_SIZE 1024\n");
    fprintf(fp, "static proof_cache_entry_t proof_cache[PROOF_CACHE_SIZE];\n");
    fprintf(fp, "static int cache_count = 0;\n\n");
    
    /* Generate verification functions */
    fprintf(fp, "/* Check if operation needs verification */\n");
    fprintf(fp, "static bool needs_verification(uint32_t op_id, bool is_deterministic) {\n");
    fprintf(fp, "    if (!is_deterministic) return true;\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    /* Check cache */\n");
    fprintf(fp, "    for (int i = 0; i < cache_count; i++) {\n");
    fprintf(fp, "        if (proof_cache[i].operation_id == op_id) {\n");
    fprintf(fp, "            return !proof_cache[i].verified;\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return true;\n");
    fprintf(fp, "}\n\n");
    
    /* Generate specific verification macros */
    fprintf(fp, "/* Optimized verification macros */\n");
    
    int eliminable = 0;
    for (int i = 0; i < analyzer->operation_count; i++) {
        const operation_optimization_t* opt = &analyzer->operations[i];
        if (opt->strategy == OPT_ELIMINATE) {
            fprintf(fp, "#define VERIFY_OP_%d() /* Eliminated - proven safe */\n", i);
            eliminable++;
        } else if (opt->strategy == OPT_CACHE) {
            fprintf(fp, "#define VERIFY_OP_%d() do { \\\n", i);
            fprintf(fp, "    if (needs_verification(%d, true)) { \\\n", i);
            fprintf(fp, "        /* Perform verification */ \\\n");
            fprintf(fp, "        proof_cache[cache_count++] = (proof_cache_entry_t){%d, true}; \\\n", i);
            fprintf(fp, "    } \\\n");
            fprintf(fp, "} while(0)\n");
        } else {
            fprintf(fp, "#define VERIFY_OP_%d() /* Runtime verification required */\n", i);
        }
    }
    
    fprintf(fp, "\n/* Total eliminated checks: %d */\n", eliminable);
    
    fclose(fp);
    return 0;
}
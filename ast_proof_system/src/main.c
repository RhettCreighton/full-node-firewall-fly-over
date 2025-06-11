/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "ast_analyzer.h"
#include "determinism_analyzer.h"
#include "constraint_verifier.h"

/* Program modes */
typedef enum {
    MODE_ANALYZE = 0,
    MODE_DETERMINISM,
    MODE_CONSTRAINTS,
    MODE_FULL_PROOF,
    MODE_HELP
} program_mode_t;

/* Print usage */
static void print_usage(const char* program_name) {
    printf("Firewall Fly-Over - Core Dump Prevention System\n");
    printf("Usage: %s [options] <source_file.c>\n\n", program_name);
    printf("Options:\n");
    printf("  -a, --analyze         Analyze AST for dangerous operations (default)\n");
    printf("  -d, --determinism     Analyze determinism and optimization potential\n");
    printf("  -c, --constraints     Verify constraints on non-deterministic inputs\n");
    printf("  -f, --full-proof      Run complete deterministic proof analysis\n");
    printf("  -o, --output FILE     Output file for generated proofs\n");
    printf("  -v, --verbose         Verbose output\n");
    printf("  -h, --help            Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s game.c                    # Basic AST analysis\n", program_name);
    printf("  %s -f game.c                 # Full proof analysis\n", program_name);
    printf("  %s -d -o opt.c game.c        # Generate optimized code\n", program_name);
}

/* Main analysis pipeline */
static int run_full_analysis(const char* source_file, const char* output_base, bool verbose) {
    char ast_file[256];
    char gdb_proof[256];
    char opt_code[256];
    char constraint_proof[256];
    
    /* Generate file names */
    snprintf(ast_file, sizeof(ast_file), "%s.ast", output_base);
    snprintf(gdb_proof, sizeof(gdb_proof), "%s_proof.gdb", output_base);
    snprintf(opt_code, sizeof(opt_code), "%s_optimized.c", output_base);
    snprintf(constraint_proof, sizeof(constraint_proof), "%s_constraints.gdb", output_base);
    
    /* Step 1: Generate AST */
    if (verbose) printf("Generating AST...\n");
    if (generate_ast(source_file, ast_file) != 0) {
        fprintf(stderr, "Failed to generate AST\n");
        return -1;
    }
    
    /* Step 2: Analyze AST for dangerous operations */
    if (verbose) printf("Analyzing AST for dangerous operations...\n");
    analysis_result_t analysis;
    if (analyze_ast(ast_file, &analysis) != 0) {
        fprintf(stderr, "Failed to analyze AST\n");
        return -1;
    }
    
    print_analysis_result(&analysis);
    
    /* Step 3: Analyze determinism */
    if (verbose) printf("\nAnalyzing determinism...\n");
    determinism_analyzer_t det_analyzer;
    init_determinism_analyzer(&det_analyzer);
    
    /* TODO: Properly parse AST and analyze each function */
    /* For now, analyze operations for optimization */
    for (int i = 0; i < analysis.dangerous_count; i++) {
        operation_optimization_t opt;
        analyze_operation_optimization(&det_analyzer, &analysis.dangerous_ops[i], &opt);
        det_analyzer.operations[det_analyzer.operation_count++] = opt;
    }
    
    det_analyzer.total_operations = analysis.dangerous_count;
    calculate_optimization_stats(&det_analyzer);
    print_determinism_analysis(&det_analyzer);
    
    /* Step 4: Verify constraints */
    if (verbose) printf("\nVerifying constraints...\n");
    constraint_verifier_t verifier;
    init_constraint_verifier(&verifier);
    
    /* Check each dangerous operation for constraints */
    for (int i = 0; i < analysis.dangerous_count; i++) {
        constraint_violation_t violation;
        if (!verify_operation_constraints(&verifier, &analysis.dangerous_ops[i], &violation)) {
            if (verifier.violation_count < MAX_VIOLATIONS) {
                verifier.violations[verifier.violation_count++] = violation;
            }
        }
    }
    
    calculate_constraint_coverage(&verifier);
    print_constraint_report(&verifier);
    
    /* Step 5: Generate outputs */
    if (verbose) printf("\nGenerating proof artifacts...\n");
    
    /* Generate GDB proof */
    if (generate_gdb_proof(&analysis, gdb_proof) == 0) {
        printf("\nGDB proof script: %s\n", gdb_proof);
    }
    
    /* Generate optimized verification code */
    if (generate_optimized_verification(&det_analyzer, opt_code) == 0) {
        printf("Optimized verification: %s\n", opt_code);
    }
    
    /* Generate constraint proof */
    if (generate_constraint_proof(&verifier, constraint_proof) == 0) {
        printf("Constraint proof: %s\n", constraint_proof);
    }
    
    /* Summary */
    printf("\n=== Proof Summary ===\n");
    printf("Safety Score: %.1f%%\n", analysis.safety_score);
    printf("Constraint Coverage: %.1f%%\n", verifier.constraint_coverage * 100);
    printf("Optimization Potential: %.1fx speedup\n", det_analyzer.opt_stats.estimated_speedup);
    
    if (analysis.unprotected_count == 0 && verifier.violation_count == 0) {
        printf("\n✓ All operations are mathematically proven safe!\n");
        return 0;
    } else {
        printf("\n✗ %d unprotected operations, %d constraint violations\n",
               analysis.unprotected_count, verifier.violation_count);
        return 1;
    }
}

int main(int argc, char* argv[]) {
    /* Parse command line options */
    static struct option long_options[] = {
        {"analyze", no_argument, 0, 'a'},
        {"determinism", no_argument, 0, 'd'},
        {"constraints", no_argument, 0, 'c'},
        {"full-proof", no_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    program_mode_t mode = MODE_ANALYZE;
    char* output_file = NULL;
    bool verbose = false;
    
    int opt;
    while ((opt = getopt_long(argc, argv, "adcfo:vh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'a':
            mode = MODE_ANALYZE;
            break;
        case 'd':
            mode = MODE_DETERMINISM;
            break;
        case 'c':
            mode = MODE_CONSTRAINTS;
            break;
        case 'f':
            mode = MODE_FULL_PROOF;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }
    
    /* Check for source file */
    if (optind >= argc) {
        fprintf(stderr, "Error: No source file specified\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    const char* source_file = argv[optind];
    
    /* Check if file exists */
    if (access(source_file, F_OK) != 0) {
        fprintf(stderr, "Error: Source file '%s' not found\n", source_file);
        return 1;
    }
    
    /* Determine output base name */
    char output_base[256];
    if (output_file) {
        strncpy(output_base, output_file, sizeof(output_base) - 1);
        /* Remove extension if present */
        char* dot = strrchr(output_base, '.');
        if (dot) *dot = '\0';
    } else {
        /* Use source file name without extension */
        strncpy(output_base, source_file, sizeof(output_base) - 1);
        char* dot = strrchr(output_base, '.');
        if (dot) *dot = '\0';
    }
    
    /* Run analysis based on mode */
    int result = 0;
    
    switch (mode) {
    case MODE_FULL_PROOF:
        result = run_full_analysis(source_file, output_base, verbose);
        break;
        
    case MODE_ANALYZE:
        {
            char ast_file[256];
            snprintf(ast_file, sizeof(ast_file), "%s.ast", output_base);
            
            if (generate_ast(source_file, ast_file) == 0) {
                analysis_result_t analysis;
                if (analyze_ast(ast_file, &analysis) == 0) {
                    print_analysis_result(&analysis);
                    
                    char gdb_proof[256];
                    snprintf(gdb_proof, sizeof(gdb_proof), "%s_proof.gdb", output_base);
                    if (generate_gdb_proof(&analysis, gdb_proof) == 0) {
                        printf("\nGDB proof script: %s\n", gdb_proof);
                    }
                }
            }
        }
        break;
        
    case MODE_DETERMINISM:
        /* TODO: Implement standalone determinism analysis */
        printf("Determinism analysis not yet implemented as standalone\n");
        printf("Use -f for full analysis\n");
        result = 1;
        break;
        
    case MODE_CONSTRAINTS:
        /* TODO: Implement standalone constraint verification */
        printf("Constraint verification not yet implemented as standalone\n");
        printf("Use -f for full analysis\n");
        result = 1;
        break;
        
    default:
        break;
    }
    
    return result;
}
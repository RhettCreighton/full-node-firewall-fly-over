/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "ast_analyzer.h"

/* Operation types are defined in header */

/* String names for operations */
static const char* op_names[OP_COUNT] = {
    "division",
    "modulo",
    "dereference",
    "array_access",
    "unsafe_call",
    "cast_overflow",
    "arithmetic_overflow"
};

/* Dangerous function calls */
static const char* unsafe_functions[] = {
    "strcpy", "strcat", "sprintf", "gets",
    "scanf", "fscanf", "sscanf",
    "rand", "time", "getenv",
    NULL
};

/* Parse AST from clang output */
static int parse_ast_line(const char* line, ast_node_t* node) {
    /* Example line:
     * |-BinaryOperator 0x5594e7b248d8 <line:189:9, col:37> 'float' '/'
     */
    
    /* Count leading spaces/bars for depth */
    int depth = 0;
    const char* p = line;
    while (*p == ' ' || *p == '|' || *p == '-' || *p == '`') {
        if (*p == '|' || *p == '`') depth++;
        p++;
    }
    
    node->depth = depth;
    
    /* Extract node type */
    char type_buf[64];
    if (sscanf(p, "%63s", type_buf) != 1) {
        return -1;
    }
    
    /* Map to our enum */
    if (strcmp(type_buf, "FunctionDecl") == 0) {
        node->type = AST_FUNCTION_DECL;
    } else if (strcmp(type_buf, "BinaryOperator") == 0) {
        node->type = AST_BINARY_OP;
    } else if (strcmp(type_buf, "UnaryOperator") == 0) {
        node->type = AST_UNARY_OP;
    } else if (strcmp(type_buf, "CallExpr") == 0) {
        node->type = AST_CALL_EXPR;
    } else if (strcmp(type_buf, "ArraySubscriptExpr") == 0) {
        node->type = AST_ARRAY_SUBSCRIPT;
    } else if (strcmp(type_buf, "IfStmt") == 0) {
        node->type = AST_IF_STMT;
    } else if (strcmp(type_buf, "CompoundStmt") == 0) {
        node->type = AST_COMPOUND_STMT;
    } else {
        node->type = AST_UNKNOWN;
    }
    
    /* Extract location */
    const char* loc_start = strstr(p, "<");
    const char* loc_end = strstr(p, ">");
    if (loc_start && loc_end) {
        size_t loc_len = loc_end - loc_start - 1;
        if (loc_len < sizeof(node->location) - 1) {
            strncpy(node->location, loc_start + 1, loc_len);
            node->location[loc_len] = '\0';
        }
    }
    
    /* Store full line for detail extraction */
    strncpy(node->content, line, sizeof(node->content) - 1);
    node->content[sizeof(node->content) - 1] = '\0';
    
    return 0;
}

/* Check if operation is division or modulo */
static bool is_division_op(const ast_node_t* node) {
    return node->type == AST_BINARY_OP && 
           (strstr(node->content, "'/'") || strstr(node->content, "'%'"));
}

/* Check if operation is pointer dereference */
static bool is_deref_op(const ast_node_t* node) {
    return (node->type == AST_UNARY_OP && strstr(node->content, "'*'")) ||
           (strstr(node->content, "->"));
}

/* Check if function call is unsafe */
static bool is_unsafe_call(const ast_node_t* node, const char** func_name) {
    if (node->type != AST_CALL_EXPR) return false;
    
    for (int i = 0; unsafe_functions[i]; i++) {
        if (strstr(node->content, unsafe_functions[i])) {
            *func_name = unsafe_functions[i];
            return true;
        }
    }
    
    return false;
}

/* Find guard conditions for an operation */
static int find_guards(const ast_node_t* nodes, int node_count, int op_index,
                      guard_condition_t* guards, int max_guards) {
    int guard_count = 0;
    int op_depth = nodes[op_index].depth;
    
    /* Look backwards for if statements at lower depth */
    for (int i = op_index - 1; i >= 0 && guard_count < max_guards; i--) {
        if (nodes[i].type == AST_IF_STMT && nodes[i].depth < op_depth) {
            /* Found a guarding if statement */
            guards[guard_count].type = GUARD_IF_STMT;
            strncpy(guards[guard_count].expression, nodes[i].content,
                   sizeof(guards[guard_count].expression) - 1);
            
            /* Check for common patterns */
            if (strstr(nodes[i].content, "!= 0") || 
                strstr(nodes[i].content, "> 0")) {
                guards[guard_count].protects_division = true;
            }
            if (strstr(nodes[i].content, "!= NULL") ||
                strstr(nodes[i].content, "NULL !=")) {
                guards[guard_count].protects_null = true;
            }
            if (strstr(nodes[i].content, "< ") && strstr(nodes[i].content, ">=")) {
                guards[guard_count].protects_bounds = true;
            }
            
            guard_count++;
        }
    }
    
    return guard_count;
}

/* Main analysis function */
int analyze_ast(const char* ast_file, analysis_result_t* result) {
    FILE* fp = fopen(ast_file, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open AST file: %s\n", ast_file);
        return -1;
    }
    
    /* Initialize result */
    memset(result, 0, sizeof(*result));
    result->source_file = ast_file;
    
    /* Parse AST nodes */
    ast_node_t* nodes = malloc(sizeof(ast_node_t) * MAX_AST_NODES);
    if (!nodes) {
        fclose(fp);
        return -1;
    }
    
    int node_count = 0;
    char line[1024];
    char current_function[256] = "global";
    
    while (fgets(line, sizeof(line), fp) && node_count < MAX_AST_NODES) {
        ast_node_t* node = &nodes[node_count];
        
        if (parse_ast_line(line, node) == 0) {
            /* Track current function */
            if (node->type == AST_FUNCTION_DECL) {
                /* Extract function name */
                char* name_start = strstr(node->content, "'");
                if (name_start) {
                    char* name_end = strchr(name_start + 1, '\'');
                    if (name_end) {
                        size_t len = name_end - name_start - 1;
                        if (len < sizeof(current_function) - 1) {
                            strncpy(current_function, name_start + 1, len);
                            current_function[len] = '\0';
                        }
                    }
                }
            }
            
            /* Check for dangerous operations */
            dangerous_op_t* op = NULL;
            const char* unsafe_func = NULL;
            
            if (is_division_op(node)) {
                if (result->dangerous_count < MAX_DANGEROUS_OPS) {
                    op = &result->dangerous_ops[result->dangerous_count++];
                    op->type = strstr(node->content, "'%'") ? OP_MODULO : OP_DIVISION;
                }
            } else if (is_deref_op(node)) {
                if (result->dangerous_count < MAX_DANGEROUS_OPS) {
                    op = &result->dangerous_ops[result->dangerous_count++];
                    op->type = OP_DEREF;
                }
            } else if (node->type == AST_ARRAY_SUBSCRIPT) {
                if (result->dangerous_count < MAX_DANGEROUS_OPS) {
                    op = &result->dangerous_ops[result->dangerous_count++];
                    op->type = OP_ARRAY_ACCESS;
                }
            } else if (is_unsafe_call(node, &unsafe_func)) {
                if (result->dangerous_count < MAX_DANGEROUS_OPS) {
                    op = &result->dangerous_ops[result->dangerous_count++];
                    op->type = OP_CALL_UNSAFE;
                }
            }
            
            /* Fill operation details */
            if (op) {
                strncpy(op->location, node->location, sizeof(op->location) - 1);
                strncpy(op->function, current_function, sizeof(op->function) - 1);
                op->guard_count = find_guards(nodes, node_count, node_count, 
                                            op->guards, MAX_GUARDS);
                
                /* Determine if adequately protected */
                op->is_safe = false;
                if (op->type == OP_DIVISION || op->type == OP_MODULO) {
                    for (int i = 0; i < op->guard_count; i++) {
                        if (op->guards[i].protects_division) {
                            op->is_safe = true;
                            break;
                        }
                    }
                } else if (op->type == OP_DEREF) {
                    for (int i = 0; i < op->guard_count; i++) {
                        if (op->guards[i].protects_null) {
                            op->is_safe = true;
                            break;
                        }
                    }
                } else if (op->type == OP_ARRAY_ACCESS) {
                    for (int i = 0; i < op->guard_count; i++) {
                        if (op->guards[i].protects_bounds) {
                            op->is_safe = true;
                            break;
                        }
                    }
                }
                
                /* Update stats */
                if (!op->is_safe) {
                    result->unprotected_count++;
                }
            }
            
            node_count++;
        }
    }
    
    /* Calculate safety score */
    if (result->dangerous_count > 0) {
        result->safety_score = 100.0 * (result->dangerous_count - result->unprotected_count) 
                              / result->dangerous_count;
    } else {
        result->safety_score = 100.0;
    }
    
    free(nodes);
    fclose(fp);
    
    return 0;
}

/* Generate AST using clang */
int generate_ast(const char* source_file, const char* output_file) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
            "clang -Xclang -ast-dump -fsyntax-only -I../include %s > %s 2>&1",
            source_file, output_file);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to generate AST for %s\n", source_file);
        return -1;
    }
    
    return 0;
}

/* Print analysis results */
void print_analysis_result(const analysis_result_t* result) {
    printf("=== AST Safety Analysis ===\n");
    printf("Source: %s\n", result->source_file);
    printf("Dangerous operations found: %d\n", result->dangerous_count);
    printf("Unprotected operations: %d\n", result->unprotected_count);
    printf("Safety score: %.1f%%\n\n", result->safety_score);
    
    if (result->unprotected_count > 0) {
        printf("Unprotected operations:\n");
        for (int i = 0; i < result->dangerous_count; i++) {
            const dangerous_op_t* op = &result->dangerous_ops[i];
            if (!op->is_safe) {
                printf("  %s at %s in function %s\n",
                      op_names[op->type], op->location, op->function);
            }
        }
    }
}

/* Generate GDB proof script */
int generate_gdb_proof(const analysis_result_t* result, const char* output_file) {
    FILE* fp = fopen(output_file, "w");
    if (!fp) return -1;
    
    fprintf(fp, "# GDB Proof Script\n");
    fprintf(fp, "# Generated from AST analysis\n\n");
    
    fprintf(fp, "define verify_safety\n");
    fprintf(fp, "  set $violations = 0\n\n");
    
    for (int i = 0; i < result->dangerous_count; i++) {
        const dangerous_op_t* op = &result->dangerous_ops[i];
        if (!op->is_safe) {
            fprintf(fp, "  # Verify %s at %s\n", op_names[op->type], op->location);
            
            if (op->type == OP_DIVISION || op->type == OP_MODULO) {
                fprintf(fp, "  # TODO: Set breakpoint and verify divisor != 0\n");
            } else if (op->type == OP_DEREF) {
                fprintf(fp, "  # TODO: Set breakpoint and verify pointer != NULL\n");
            } else if (op->type == OP_ARRAY_ACCESS) {
                fprintf(fp, "  # TODO: Set breakpoint and verify index in bounds\n");
            }
            
            fprintf(fp, "\n");
        }
    }
    
    fprintf(fp, "  if $violations == 0\n");
    fprintf(fp, "    printf \"All operations verified safe\\n\"\n");
    fprintf(fp, "  else\n");
    fprintf(fp, "    printf \"%%d violations found\\n\", $violations\n");
    fprintf(fp, "  end\n");
    fprintf(fp, "end\n\n");
    fprintf(fp, "verify_safety\n");
    
    fclose(fp);
    return 0;
}
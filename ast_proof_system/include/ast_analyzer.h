/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AST_ANALYZER_H
#define AST_ANALYZER_H

#include <stdbool.h>

/* Maximum limits */
#define MAX_AST_NODES 10000
#define MAX_DANGEROUS_OPS 1000
#define MAX_GUARDS 10
#define MAX_LOCATION_LEN 256
#define MAX_FUNCTION_LEN 128

/* AST node types */
enum {
    AST_UNKNOWN = 0,
    AST_BINARY_OP = 1,
    AST_UNARY_OP = 2,
    AST_CALL_EXPR = 3,
    AST_ARRAY_SUBSCRIPT = 4,
    AST_FUNCTION_DECL = 5,
    AST_IF_STMT = 6,
    AST_COMPOUND_STMT = 7
};

/* AST node structure */
typedef struct {
    int type;
    int depth;
    char location[MAX_LOCATION_LEN];
    char content[1024];
} ast_node_t;

/* Operation types */
enum {
    OP_DIVISION = 0,
    OP_MODULO,
    OP_DEREF,
    OP_ARRAY_ACCESS,
    OP_CALL_UNSAFE,
    OP_CAST_OVERFLOW,
    OP_ARITHMETIC_OVERFLOW,
    OP_COUNT
};

/* Guard condition */
typedef struct {
    int type;
    char expression[512];
    bool protects_division;
    bool protects_null;
    bool protects_bounds;
} guard_condition_t;

/* Guard types */
enum {
    GUARD_NONE = 0,
    GUARD_IF_STMT,
    GUARD_TERNARY,
    GUARD_ASSERT,
    GUARD_EARLY_RETURN
};

/* Dangerous operation */
typedef struct {
    int type;
    char location[MAX_LOCATION_LEN];
    char function[MAX_FUNCTION_LEN];
    guard_condition_t guards[MAX_GUARDS];
    int guard_count;
    bool is_safe;
} dangerous_op_t;

/* Analysis result */
typedef struct {
    const char* source_file;
    dangerous_op_t dangerous_ops[MAX_DANGEROUS_OPS];
    int dangerous_count;
    int unprotected_count;
    double safety_score;
} analysis_result_t;

/* Functions */
int generate_ast(const char* source_file, const char* output_file);
int analyze_ast(const char* ast_file, analysis_result_t* result);
void print_analysis_result(const analysis_result_t* result);
int generate_gdb_proof(const analysis_result_t* result, const char* output_file);

#endif /* AST_ANALYZER_H */
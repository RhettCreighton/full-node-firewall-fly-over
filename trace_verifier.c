/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TRACES 10000
#define MAX_TRACE_NAME 256
#define MAX_SPECS 100

typedef struct {
    char name[MAX_TRACE_NAME];
    char sha3_tag[65]; /* 32 bytes as hex = 64 chars + null */
} trace_entry_t;

typedef struct {
    char spec_name[256];
    char required_traces[100][MAX_TRACE_NAME];
    int required_count;
    char forbidden_traces[100][MAX_TRACE_NAME];
    int forbidden_count;
} specification_t;

typedef struct {
    trace_entry_t traces[MAX_TRACES];
    int trace_count;
    specification_t specs[MAX_SPECS];
    int spec_count;
} verification_context_t;

/* Parse trace output line */
int parse_trace_line(const char* line, trace_entry_t* entry) {
    /* Format: TRACE:NAME:sha3tag */
    if (strncmp(line, "TRACE:", 6) != 0) {
        return 0;
    }
    
    const char* name_start = line + 6;
    const char* colon = strchr(name_start, ':');
    if (!colon) return 0;
    
    int name_len = colon - name_start;
    if (name_len >= MAX_TRACE_NAME) return 0;
    
    strncpy(entry->name, name_start, name_len);
    entry->name[name_len] = '\0';
    
    /* Copy SHA3 tag */
    strncpy(entry->sha3_tag, colon + 1, 64);
    entry->sha3_tag[64] = '\0';
    
    return 1;
}

/* Load traces from file */
int load_traces(const char* filename, verification_context_t* ctx) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open trace file");
        return -1;
    }
    
    char line[1024];
    ctx->trace_count = 0;
    
    while (fgets(line, sizeof(line), fp) && ctx->trace_count < MAX_TRACES) {
        trace_entry_t entry;
        if (parse_trace_line(line, &entry)) {
            ctx->traces[ctx->trace_count++] = entry;
        }
    }
    
    fclose(fp);
    return ctx->trace_count;
}

/* Check if trace exists in list */
bool trace_exists(verification_context_t* ctx, const char* name) {
    for (int i = 0; i < ctx->trace_count; i++) {
        if (strcmp(ctx->traces[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

/* Check trace order */
bool check_trace_order(verification_context_t* ctx, const char* first, const char* second) {
    int first_idx = -1;
    int second_idx = -1;
    
    for (int i = 0; i < ctx->trace_count; i++) {
        if (strcmp(ctx->traces[i].name, first) == 0 && first_idx == -1) {
            first_idx = i;
        }
        if (strcmp(ctx->traces[i].name, second) == 0 && first_idx != -1) {
            second_idx = i;
            break;
        }
    }
    
    return first_idx != -1 && second_idx != -1 && first_idx < second_idx;
}

/* Verify specification */
bool verify_specification(verification_context_t* ctx, specification_t* spec) {
    printf("\nVerifying: %s\n", spec->spec_name);
    
    /* Check all required traces exist */
    for (int i = 0; i < spec->required_count; i++) {
        if (!trace_exists(ctx, spec->required_traces[i])) {
            printf("  ❌ FAILED: Required trace '%s' not found\n", 
                   spec->required_traces[i]);
            return false;
        }
        printf("  ✓ Found required trace: %s\n", spec->required_traces[i]);
    }
    
    /* Check forbidden traces don't exist */
    for (int i = 0; i < spec->forbidden_count; i++) {
        if (trace_exists(ctx, spec->forbidden_traces[i])) {
            printf("  ❌ FAILED: Forbidden trace '%s' found!\n", 
                   spec->forbidden_traces[i]);
            return false;
        }
        printf("  ✓ Forbidden trace absent: %s\n", spec->forbidden_traces[i]);
    }
    
    /* Check order if we have multiple required traces */
    if (spec->required_count > 1) {
        for (int i = 0; i < spec->required_count - 1; i++) {
            if (!check_trace_order(ctx, spec->required_traces[i], 
                                  spec->required_traces[i+1])) {
                printf("  ❌ FAILED: Trace order violation\n");
                return false;
            }
        }
        printf("  ✓ Trace order correct\n");
    }
    
    printf("  ✅ PASSED: %s\n", spec->spec_name);
    return true;
}

/* Add a specification programmatically */
void add_specification(verification_context_t* ctx, const char* name,
                      const char* required[], int req_count,
                      const char* forbidden[], int forb_count) {
    if (ctx->spec_count >= MAX_SPECS) return;
    
    specification_t* spec = &ctx->specs[ctx->spec_count++];
    strcpy(spec->spec_name, name);
    
    spec->required_count = req_count;
    for (int i = 0; i < req_count; i++) {
        strcpy(spec->required_traces[i], required[i]);
    }
    
    spec->forbidden_count = forb_count;
    for (int i = 0; i < forb_count; i++) {
        strcpy(spec->forbidden_traces[i], forbidden[i]);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <trace_file> [spec_name]\n", argv[0]);
        return 1;
    }
    
    verification_context_t ctx = {0};
    
    /* Load traces */
    int trace_count = load_traces(argv[1], &ctx);
    if (trace_count < 0) {
        return 1;
    }
    
    printf("Loaded %d traces\n", trace_count);
    
    /* Define specifications */
    const char* world_req[] = {"WORLD_INIT_START", "WORLD_CHUNK_LOADED", "WORLD_SEAMLESS_READY"};
    const char* world_forb[] = {"LOADING_SCREEN_SHOWN", "WORLD_BOUNDARY_HIT"};
    add_specification(&ctx, "Seamless Open World", world_req, 3, world_forb, 2);
    
    const char* physics_req[] = {"PHYSICS_UPDATE_START", "CALCULATE_LIFT_FORCE", "APPLY_AERODYNAMICS"};
    const char* physics_forb[] = {"PHYSICS_TELEPORT", "PHYSICS_INFINITE_ACCELERATION"};
    add_specification(&ctx, "Realistic Physics", physics_req, 3, physics_forb, 2);
    
    const char* ai_req[] = {"AI_SPAWN_CHARACTER", "AI_ASSIGN_PERSONALITY", "AI_BEGIN_ROUTINE"};
    const char* ai_forb[] = {"AI_WALK_THROUGH_BUILDING", "AI_IDENTICAL_BEHAVIOR"};
    add_specification(&ctx, "AI Characters", ai_req, 3, ai_forb, 2);
    
    /* Verify all or specific spec */
    bool all_passed = true;
    if (argc > 2) {
        /* Verify specific spec */
        for (int i = 0; i < ctx.spec_count; i++) {
            if (strstr(ctx.specs[i].spec_name, argv[2])) {
                if (!verify_specification(&ctx, &ctx.specs[i])) {
                    all_passed = false;
                }
                break;
            }
        }
    } else {
        /* Verify all specs */
        for (int i = 0; i < ctx.spec_count; i++) {
            if (!verify_specification(&ctx, &ctx.specs[i])) {
                all_passed = false;
            }
        }
    }
    
    printf("\n%s\n", all_passed ? "ALL SPECIFICATIONS PASSED!" : "SOME SPECIFICATIONS FAILED!");
    return all_passed ? 0 : 1;
}
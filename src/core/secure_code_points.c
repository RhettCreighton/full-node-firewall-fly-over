/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/core/secure_code_points.h"
#include <stdio.h>

#ifndef PROVING_BUILD

/* Record of reached code points during normal execution */
typedef struct {
    char name[64];
    secure_tag_t tag;
    char condition[128];
    int hit_count;
} code_point_record_t;

static code_point_record_t reached_points[1024];
static int num_reached = 0;

void record_secure_point(const char* name, const secure_tag_t* tag, const char* condition) {
    /* Check if already recorded */
    for (int i = 0; i < num_reached; i++) {
        if (strcmp(reached_points[i].name, name) == 0) {
            reached_points[i].hit_count++;
            return;
        }
    }
    
    /* New code point */
    if (num_reached < 1024) {
        strncpy(reached_points[num_reached].name, name, 63);
        reached_points[num_reached].tag = *tag;
        strncpy(reached_points[num_reached].condition, condition, 127);
        reached_points[num_reached].hit_count = 1;
        num_reached++;
    }
}

void dump_reachability_proof(void) {
    printf("\n=== REACHABILITY PROOF ===\n");
    printf("Code points reached: %d\n\n", num_reached);
    
    for (int i = 0; i < num_reached; i++) {
        printf("Point: %s\n", reached_points[i].name);
        printf("  Condition: %s\n", reached_points[i].condition);
        printf("  Hit count: %d\n", reached_points[i].hit_count);
        printf("  SHA3 tag: ");
        for (int j = 0; j < 32; j++) {
            printf("%02x", reached_points[i].tag.bytes[j]);
        }
        printf("\n\n");
    }
}

#endif /* !PROVING_BUILD */
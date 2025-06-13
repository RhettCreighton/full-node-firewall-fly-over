/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATHS 1000
#define MAX_PATH_NAME 256
#define MAX_CMD_LEN 8192

typedef struct {
    char name[MAX_PATH_NAME];
    int exists;
} path_config_t;

typedef struct {
    char source_file[256];
    char output_binary[256];
    path_config_t paths[MAX_PATHS];
    int path_count;
    int proving_build;
    int tracing_build;
} build_config_t;

/* Extract all SECURE_CODE_POINT names from source */
int extract_code_points(const char* filename, build_config_t* config) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open source file");
        return -1;
    }
    
    char line[1024];
    config->path_count = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        char* pos = strstr(line, "SECURE_CODE_POINT(");
        if (pos) {
            pos += strlen("SECURE_CODE_POINT(");
            char* end = strchr(pos, ',');
            if (end) {
                int len = end - pos;
                if (len < MAX_PATH_NAME - 1) {
                    strncpy(config->paths[config->path_count].name, pos, len);
                    config->paths[config->path_count].name[len] = '\0';
                    config->paths[config->path_count].exists = 1; /* Default enabled */
                    config->path_count++;
                }
            }
        }
    }
    
    fclose(fp);
    return config->path_count;
}

/* Build command with path definitions */
void build_proof_binary(build_config_t* config) {
    char cmd[MAX_CMD_LEN];
    int offset = 0;
    
    /* Start with compiler and flags */
    offset += snprintf(cmd + offset, MAX_CMD_LEN - offset, 
                      "gcc -std=c99 -I./include");
    
    /* Add build mode */
    if (config->proving_build) {
        offset += snprintf(cmd + offset, MAX_CMD_LEN - offset, " -DPROVING_BUILD");
    } else if (config->tracing_build) {
        offset += snprintf(cmd + offset, MAX_CMD_LEN - offset, " -DTRACING_BUILD");
    }
    
    /* Add all path definitions */
    for (int i = 0; i < config->path_count; i++) {
        offset += snprintf(cmd + offset, MAX_CMD_LEN - offset, 
                          " -DPATH_%s_EXISTS=%d",
                          config->paths[i].name, 
                          config->paths[i].exists);
    }
    
    /* Add source and output */
    offset += snprintf(cmd + offset, MAX_CMD_LEN - offset, 
                      " %s -o %s -lm",
                      config->source_file,
                      config->output_binary);
    
    printf("Building: %s\n", cmd);
    system(cmd);
}

/* Enable specific paths by name */
void enable_paths(build_config_t* config, const char* path_list) {
    /* First disable all */
    for (int i = 0; i < config->path_count; i++) {
        config->paths[i].exists = 0;
    }
    
    /* Parse comma-separated list */
    char* list_copy = strdup(path_list);
    char* token = strtok(list_copy, ",");
    
    while (token) {
        /* Find and enable this path */
        for (int i = 0; i < config->path_count; i++) {
            if (strcmp(config->paths[i].name, token) == 0) {
                config->paths[i].exists = 1;
                break;
            }
        }
        token = strtok(NULL, ",");
    }
    
    free(list_copy);
}

/* Disable specific paths by name */
void disable_paths(build_config_t* config, const char* path_list) {
    char* list_copy = strdup(path_list);
    char* token = strtok(list_copy, ",");
    
    while (token) {
        /* Find and disable this path */
        for (int i = 0; i < config->path_count; i++) {
            if (strcmp(config->paths[i].name, token) == 0) {
                config->paths[i].exists = 0;
                break;
            }
        }
        token = strtok(NULL, ",");
    }
    
    free(list_copy);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <source.c> <output> [options]\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  --proving        Build for proof mode (exits at points)\n");
        fprintf(stderr, "  --tracing        Build for trace mode (records paths)\n");
        fprintf(stderr, "  --enable <paths> Enable only these paths (comma-separated)\n");
        fprintf(stderr, "  --disable <paths> Disable these paths (comma-separated)\n");
        return 1;
    }
    
    build_config_t config = {0};
    strcpy(config.source_file, argv[1]);
    strcpy(config.output_binary, argv[2]);
    
    /* Extract all code points from source */
    if (extract_code_points(config.source_file, &config) < 0) {
        return 1;
    }
    
    printf("Found %d secure code points\n", config.path_count);
    
    /* Parse options */
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--proving") == 0) {
            config.proving_build = 1;
        } else if (strcmp(argv[i], "--tracing") == 0) {
            config.tracing_build = 1;
        } else if (strcmp(argv[i], "--enable") == 0 && i + 1 < argc) {
            enable_paths(&config, argv[++i]);
        } else if (strcmp(argv[i], "--disable") == 0 && i + 1 < argc) {
            disable_paths(&config, argv[++i]);
        }
    }
    
    /* Build the binary */
    build_proof_binary(&config);
    
    return 0;
}
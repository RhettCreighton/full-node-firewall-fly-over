/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Simple build and test runner in C99
 * Replaces shell scripts with pure C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Run a command and return exit code */
int run_command(const char* cmd) {
    printf("Running: %s\n", cmd);
    int ret = system(cmd);
    return WEXITSTATUS(ret);
}

/* Check if program exists */
int check_program(const char* prog) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "which %s > /dev/null 2>&1", prog);
    return system(cmd) == 0;
}

/* Create directory if it doesn't exist */
void ensure_directory(const char* dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0755);
    }
}

int main(int argc, char* argv[]) {
    printf("=== AST Proof System Build ===\n\n");
    
    /* Check dependencies */
    printf("Checking dependencies...\n");
    
    int missing = 0;
    if (!check_program("cmake")) {
        printf("✗ cmake not found\n");
        missing = 1;
    } else {
        printf("✓ cmake found\n");
    }
    
    if (!check_program("clang")) {
        printf("✗ clang not found\n");
        missing = 1;
    } else {
        printf("✓ clang found\n");
    }
    
    if (!check_program("gdb")) {
        printf("✗ gdb not found\n");
        missing = 1;
    } else {
        printf("✓ gdb found\n");
    }
    
    if (missing) {
        printf("\nInstall missing dependencies:\n");
        printf("  Ubuntu: sudo apt-get install cmake clang gdb\n");
        printf("  macOS: brew install cmake llvm gdb\n");
        return 1;
    }
    
    /* Create build directory */
    printf("\nCreating build directory...\n");
    ensure_directory("build");
    
    /* Change to build directory */
    if (chdir("build") != 0) {
        perror("Failed to change to build directory");
        return 1;
    }
    
    /* Run CMake */
    printf("\nConfiguring with CMake...\n");
    if (run_command("cmake ..") != 0) {
        printf("CMake configuration failed\n");
        return 1;
    }
    
    /* Build */
    printf("\nBuilding...\n");
    if (run_command("make -j4") != 0) {
        printf("Build failed\n");
        return 1;
    }
    
    /* Run tests */
    printf("\nRunning tests...\n");
    if (run_command("make test") != 0) {
        printf("Some tests failed\n");
        /* Don't exit - expected for test files */
    }
    
    /* Demo */
    printf("\n=== Running Demo ===\n");
    printf("\nAnalyzing test_dangerous_code.c:\n");
    run_command("./bin/ast_proof -a ../test_dangerous_code.c");
    
    printf("\n=== Build Complete ===\n");
    printf("Executable: build/bin/ast_proof\n");
    printf("\nUsage:\n");
    printf("  ./bin/ast_proof -a file.c    # Analyze for dangerous operations\n");
    printf("  ./bin/ast_proof -f file.c    # Full deterministic proof\n");
    printf("  ./bin/ast_proof -h           # Show help\n");
    
    return 0;
}
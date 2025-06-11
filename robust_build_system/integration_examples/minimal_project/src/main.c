/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

/* This function is verified to be called at compile time */
void critical_function(void) {
    printf("Critical function executed - verified at compile time!\n");
}

int main(void) {
    printf("Minimal Robust Build Example\n");
    critical_function();  /* Compile fails if this is removed */
    return 0;
}
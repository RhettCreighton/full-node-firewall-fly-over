/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

/* Safety utility functions - memory and string operations */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* Safe memory allocation with failure handling */
void *safe_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "FATAL: Memory allocation failed for %zu bytes\n", size);
        abort();
    }
    
    return ptr;
}

/* Safe calloc */
void *safe_calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) {
        return NULL;
    }
    
    /* Check for overflow */
    if (nmemb > SIZE_MAX / size) {
        fprintf(stderr, "FATAL: Calloc overflow detected\n");
        abort();
    }
    
    void *ptr = calloc(nmemb, size);
    if (!ptr) {
        fprintf(stderr, "FATAL: Memory allocation failed\n");
        abort();
    }
    
    return ptr;
}

/* Safe free */
void safe_free(void *ptr) {
    free(ptr);
    /* Can't nullify the pointer here, but caller should */
}

/* Safe string copy - NEVER use strcpy! */
char *safe_strncpy(char *dest, const char *src, size_t n) {
    if (n == 0) {
        return dest;
    }
    
    /* Use strncpy but ensure null termination */
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\0';
    
    return dest;
}

/* Safe string length with maximum */
size_t safe_strnlen(const char *s, size_t maxlen) {
    const char *end = memchr(s, '\0', maxlen);
    return end ? (size_t)(end - s) : maxlen;
}

/* Safe string concatenation */
char *safe_strncat(char *dest, const char *src, size_t dest_size) {
    size_t dest_len = safe_strnlen(dest, dest_size);
    
    if (dest_len >= dest_size - 1) {
        /* No room to append */
        return dest;
    }
    
    size_t remain = dest_size - dest_len - 1;
    strncat(dest, src, remain);
    
    /* Ensure null termination */
    dest[dest_size - 1] = '\0';
    
    return dest;
}

/* Integer overflow safe operations */
bool safe_size_add(size_t a, size_t b, size_t *result) {
    if (a > SIZE_MAX - b) {
        return false;
    }
    *result = a + b;
    return true;
}

bool safe_size_multiply(size_t a, size_t b, size_t *result) {
    if (a > 0 && b > SIZE_MAX / a) {
        return false;
    }
    *result = a * b;
    return true;
}
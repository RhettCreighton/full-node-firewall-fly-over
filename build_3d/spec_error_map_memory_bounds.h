/* Specification to Error Code Mapping */
#ifndef SPEC_ERROR_MAP_memory_bounds_H
#define SPEC_ERROR_MAP_memory_bounds_H

/* Specification: memory_bounds */
#define ERROR_memory_bounds 1003

/* Compile-time verification of error code */
_Static_assert(ERROR_memory_bounds >= 1000 && ERROR_memory_bounds < 10000,
    "Invalid error code for memory_bounds");

/* Redundant expression of the mapping */
enum { memory_bounds_ERROR = 1003 };
#define memory_bounds_ERROR_CODE 1003
static const int kmemory_boundsError = 1003;

/* All expressions must be equal */
_Static_assert(ERROR_memory_bounds == memory_bounds_ERROR &&
               ERROR_memory_bounds == memory_bounds_ERROR_CODE &&
               ERROR_memory_bounds == kmemory_boundsError,
    "Redundant error mappings don't match for memory_bounds");

#endif /* SPEC_ERROR_MAP_memory_bounds_H */

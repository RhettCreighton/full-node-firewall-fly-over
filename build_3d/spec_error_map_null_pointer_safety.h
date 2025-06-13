/* Specification to Error Code Mapping */
#ifndef SPEC_ERROR_MAP_null_pointer_safety_H
#define SPEC_ERROR_MAP_null_pointer_safety_H

/* Specification: null_pointer_safety */
#define ERROR_null_pointer_safety 1002

/* Compile-time verification of error code */
_Static_assert(ERROR_null_pointer_safety >= 1000 && ERROR_null_pointer_safety < 10000,
    "Invalid error code for null_pointer_safety");

/* Redundant expression of the mapping */
enum { null_pointer_safety_ERROR = 1002 };
#define null_pointer_safety_ERROR_CODE 1002
static const int knull_pointer_safetyError = 1002;

/* All expressions must be equal */
_Static_assert(ERROR_null_pointer_safety == null_pointer_safety_ERROR &&
               ERROR_null_pointer_safety == null_pointer_safety_ERROR_CODE &&
               ERROR_null_pointer_safety == knull_pointer_safetyError,
    "Redundant error mappings don't match for null_pointer_safety");

#endif /* SPEC_ERROR_MAP_null_pointer_safety_H */

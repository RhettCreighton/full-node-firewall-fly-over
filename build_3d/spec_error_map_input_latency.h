/* Specification to Error Code Mapping */
#ifndef SPEC_ERROR_MAP_input_latency_H
#define SPEC_ERROR_MAP_input_latency_H

/* Specification: input_latency */
#define ERROR_input_latency 1005

/* Compile-time verification of error code */
_Static_assert(ERROR_input_latency >= 1000 && ERROR_input_latency < 10000,
    "Invalid error code for input_latency");

/* Redundant expression of the mapping */
enum { input_latency_ERROR = 1005 };
#define input_latency_ERROR_CODE 1005
static const int kinput_latencyError = 1005;

/* All expressions must be equal */
_Static_assert(ERROR_input_latency == input_latency_ERROR &&
               ERROR_input_latency == input_latency_ERROR_CODE &&
               ERROR_input_latency == kinput_latencyError,
    "Redundant error mappings don't match for input_latency");

#endif /* SPEC_ERROR_MAP_input_latency_H */

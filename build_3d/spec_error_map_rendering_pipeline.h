/* Specification to Error Code Mapping */
#ifndef SPEC_ERROR_MAP_rendering_pipeline_H
#define SPEC_ERROR_MAP_rendering_pipeline_H

/* Specification: rendering_pipeline */
#define ERROR_rendering_pipeline 1006

/* Compile-time verification of error code */
_Static_assert(ERROR_rendering_pipeline >= 1000 && ERROR_rendering_pipeline < 10000,
    "Invalid error code for rendering_pipeline");

/* Redundant expression of the mapping */
enum { rendering_pipeline_ERROR = 1006 };
#define rendering_pipeline_ERROR_CODE 1006
static const int krendering_pipelineError = 1006;

/* All expressions must be equal */
_Static_assert(ERROR_rendering_pipeline == rendering_pipeline_ERROR &&
               ERROR_rendering_pipeline == rendering_pipeline_ERROR_CODE &&
               ERROR_rendering_pipeline == krendering_pipelineError,
    "Redundant error mappings don't match for rendering_pipeline");

#endif /* SPEC_ERROR_MAP_rendering_pipeline_H */

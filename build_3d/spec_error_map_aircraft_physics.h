/* Specification to Error Code Mapping */
#ifndef SPEC_ERROR_MAP_aircraft_physics_H
#define SPEC_ERROR_MAP_aircraft_physics_H

/* Specification: aircraft_physics */
#define ERROR_aircraft_physics 1004

/* Compile-time verification of error code */
_Static_assert(ERROR_aircraft_physics >= 1000 && ERROR_aircraft_physics < 10000,
    "Invalid error code for aircraft_physics");

/* Redundant expression of the mapping */
enum { aircraft_physics_ERROR = 1004 };
#define aircraft_physics_ERROR_CODE 1004
static const int kaircraft_physicsError = 1004;

/* All expressions must be equal */
_Static_assert(ERROR_aircraft_physics == aircraft_physics_ERROR &&
               ERROR_aircraft_physics == aircraft_physics_ERROR_CODE &&
               ERROR_aircraft_physics == kaircraft_physicsError,
    "Redundant error mappings don't match for aircraft_physics");

#endif /* SPEC_ERROR_MAP_aircraft_physics_H */

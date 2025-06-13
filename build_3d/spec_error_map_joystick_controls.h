/* Specification to Error Code Mapping */
#ifndef SPEC_ERROR_MAP_joystick_controls_H
#define SPEC_ERROR_MAP_joystick_controls_H

/* Specification: joystick_controls */
#define ERROR_joystick_controls 1001

/* Compile-time verification of error code */
_Static_assert(ERROR_joystick_controls >= 1000 && ERROR_joystick_controls < 10000,
    "Invalid error code for joystick_controls");

/* Redundant expression of the mapping */
enum { joystick_controls_ERROR = 1001 };
#define joystick_controls_ERROR_CODE 1001
static const int kjoystick_controlsError = 1001;

/* All expressions must be equal */
_Static_assert(ERROR_joystick_controls == joystick_controls_ERROR &&
               ERROR_joystick_controls == joystick_controls_ERROR_CODE &&
               ERROR_joystick_controls == kjoystick_controlsError,
    "Redundant error mappings don't match for joystick_controls");

#endif /* SPEC_ERROR_MAP_joystick_controls_H */

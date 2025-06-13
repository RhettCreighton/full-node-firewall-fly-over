/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/core/specification.h"
#include "sky_combat/models/aircraft.h"
#include "sky_combat/core/secure_code_points.h"
#include "sky_combat/specifications/spec_init_order.h"
#include <math.h>
#include <stdlib.h>

typedef struct {
    specification_t* spec;
    bool initialized;
    
    // Control requirements
    float required_turn_rate;
    float required_pitch_rate;
    float required_roll_response;
    
    // Button mapping requirements
    int ur_button;  // Must be speed up
    int ul_button;  // Must be boost
    
    // Measurement data
    float last_yaw;
    float last_pitch;
    float last_roll;
    float last_speed;
    
} aircraft_controls_spec_t;

aircraft_controls_spec_t* aircraft_controls_spec_create(void) {
    // Ensure spec system is initialized
    spec_init_order_ensure_system_ready();
    
    aircraft_controls_spec_t* acs = calloc(1, sizeof(aircraft_controls_spec_t));
    if (!acs) return NULL;
    
    // Create the specification
    acs->spec = spec_create(
        "Aircraft Responsive Controls",
        "Aircraft must use responsive controls with proper button mapping"
    );
    
    // This is critical - wrong controls make game unplayable
    spec_set_critical(acs->spec, true);
    spec_register(acs->spec);
    
    // Set required control parameters (from aircraft_responsive.c)
    acs->required_turn_rate = 180.0f;  // degrees per second
    acs->required_pitch_rate = 12.0f;  // lerp rate for instant response
    acs->required_roll_response = 1.0f; // Direct assignment, no smoothing
    
    // Button mapping
    acs->ur_button = 3;  // UR = button 3 = gas/go
    acs->ul_button = 2;  // UL = button 2 = nitro
    
    acs->initialized = true;
    
    SECURE_CODE_POINT(RESPONSIVE_CONTROLS_SPEC_CREATED, 
                      "specification created");
    
    return acs;
}

bool aircraft_controls_spec_check_turn_rate(aircraft_controls_spec_t* acs,
                                           aircraft_t* aircraft,
                                           float stick_input,
                                           float dt) {
    if (!acs || !acs->initialized) return false;
    
    float current_yaw = aircraft->yaw;
    float yaw_change = fabs(current_yaw - acs->last_yaw);
    
    // Handle wrap-around
    if (yaw_change > 180.0f) {
        yaw_change = 360.0f - yaw_change;
    }
    
    float actual_turn_rate = yaw_change / dt;
    
    // If stick is pushed, turn rate must meet minimum
    if (fabs(stick_input) > 0.5f) {
        SPEC_CHECK(acs->spec, actual_turn_rate >= acs->required_turn_rate * 0.8f,
                   "Turn rate too slow for responsive controls");
        
        SECURE_CODE_POINT(TURN_RATE_VERIFIED, 
                          actual_turn_rate >= acs->required_turn_rate * 0.8f ? "true" : "false");
    }
    
    acs->last_yaw = current_yaw;
    return true;
}

bool aircraft_controls_spec_check_pitch_response(aircraft_controls_spec_t* acs,
                                                aircraft_t* aircraft,
                                                float target_pitch,
                                                float dt) {
    if (!acs || !acs->initialized) return false;
    
    float current_pitch = aircraft->pitch;
    float pitch_change = fabs(current_pitch - acs->last_pitch);
    
    // Check that pitch changes quickly (high lerp rate)
    if (fabs(target_pitch - current_pitch) > 5.0f) {
        float expected_change = fabs(target_pitch - acs->last_pitch) * acs->required_pitch_rate * dt;
        
        SPEC_CHECK(acs->spec, pitch_change >= expected_change * 0.7f,
                   "Pitch response too sluggish");
        
        SECURE_CODE_POINT(PITCH_RESPONSE_VERIFIED,
                          pitch_change >= expected_change * 0.7f ? "true" : "false");
    }
    
    acs->last_pitch = current_pitch;
    return true;
}

bool aircraft_controls_spec_check_roll_direct(aircraft_controls_spec_t* acs,
                                             aircraft_t* aircraft,
                                             float stick_input) {
    if (!acs || !acs->initialized) return false;
    
    // Roll should directly match stick input (no smoothing)
    float expected_roll = stick_input * 80.0f;  // MAX_ROLL from responsive
    float roll_error = fabs(aircraft->roll - expected_roll);
    
    SPEC_CHECK(acs->spec, roll_error < 5.0f,
               "Roll not directly following stick input");
    
    SECURE_CODE_POINT(ROLL_DIRECT_CONTROL,
                      roll_error < 5.0f ? "true" : "false");
    
    return roll_error < 5.0f;
}

bool aircraft_controls_spec_check_button_mapping(aircraft_controls_spec_t* acs,
                                                bool ur_pressed,
                                                bool ul_pressed,
                                                float speed_before,
                                                float speed_after,
                                                float boost_timer) {
    if (!acs || !acs->initialized) return false;
    
    // UR pressed = gas pedal, speed should gradually increase
    if (ur_pressed) {
        SPEC_CHECK(acs->spec, speed_after > speed_before,
                   "UR button not acting as gas pedal");
        
        SPEC_CHECK(acs->spec, boost_timer <= 0,
                   "UR button triggering nitro instead of gas");
        
        SECURE_CODE_POINT(UR_GAS_CORRECT,
                          speed_after > speed_before && boost_timer <= 0 ? "true" : "false");
    }
    
    // UL pressed = nitro burst should activate
    if (ul_pressed) {
        SPEC_CHECK(acs->spec, boost_timer > 0,
                   "UL button not activating nitro");
        
        SECURE_CODE_POINT(UL_NITRO_CORRECT,
                          boost_timer > 0 ? "true" : "false");
    }
    
    return true;
}

bool aircraft_controls_spec_verify_responsive_update(aircraft_controls_spec_t* acs,
                                                    const char* function_name) {
    if (!acs || !acs->initialized) return false;
    
    // Verify we're using the responsive update function
    bool is_responsive = (strstr(function_name, "responsive") != NULL);
    
    SPEC_CHECK(acs->spec, is_responsive,
               "Not using aircraft_update_responsive function");
    
    SECURE_CODE_POINT(USING_RESPONSIVE_UPDATE,
                      is_responsive ? "true" : "false");
    
    return is_responsive;
}

void aircraft_controls_spec_destroy(aircraft_controls_spec_t* acs) {
    if (!acs) return;
    
    spec_finalize(acs->spec);
    spec_report(acs->spec);
    free(acs);
}
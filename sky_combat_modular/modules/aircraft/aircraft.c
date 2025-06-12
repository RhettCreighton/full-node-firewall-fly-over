/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/aircraft/aircraft.h"
#include <stdlib.h>
#include <math.h>
#include <raymath.h>

aircraft_t* aircraft_create(Vector3 start_pos) {
    aircraft_t* aircraft = calloc(1, sizeof(aircraft_t));
    if (!aircraft) return NULL;
    
    aircraft->position = start_pos;
    aircraft->velocity = (Vector3){0, 0, 0};
    aircraft->yaw = 0;
    aircraft->pitch = 0;
    aircraft->roll = 0;
    
    aircraft->speed = AIRCRAFT_BASE_SPEED;
    aircraft->actual_speed = AIRCRAFT_BASE_SPEED;
    aircraft->boosting = false;
    aircraft->braking = false;
    aircraft->drifting = false;
    aircraft->drift_factor = 1.0f;
    aircraft->drift_velocity = (Vector3){0, 0, 0};
    
    aircraft->health = 100;
    aircraft->max_health = 100;
    aircraft->armor = 0;
    aircraft->shield = 0;
    
    aircraft->gun_cooldown = 0;
    aircraft->missile_cooldown = 0;
    aircraft->missile_count = 50;
    aircraft->gun_aim_x = 0;
    aircraft->gun_aim_y = 0;
    
    aircraft->primary_color = BLUE;
    aircraft->accent_color = WHITE;
    aircraft->engine_glow = 1.0f;
    aircraft->damage_flash = 0;
    
    aircraft->score = 0;
    aircraft->kills = 0;
    aircraft->flight_time = 0;
    aircraft->distance_traveled = 0;
    
    return aircraft;
}

void aircraft_destroy(aircraft_t* aircraft) {
    free(aircraft);
}

void aircraft_reset(aircraft_t* aircraft, Vector3 pos) {
    aircraft->position = pos;
    aircraft->velocity = (Vector3){0, 0, 0};
    aircraft->yaw = 0;
    aircraft->pitch = 0;
    aircraft->roll = 0;
    aircraft->speed = AIRCRAFT_BASE_SPEED;
    aircraft->health = aircraft->max_health;
    aircraft->damage_flash = 0;
}

void aircraft_update_controls(aircraft_t* aircraft, 
                            float stick_x, float stick_y,
                            float aim_x, float aim_y,
                            bool boost, bool brake,
                            bool drift_left, bool drift_right,
                            float dt) {
    // Store control states
    aircraft->boosting = boost;
    aircraft->braking = brake;
    aircraft->gun_aim_x = aim_x * 15.0f;  // ±15 degrees
    aircraft->gun_aim_y = aim_y * 15.0f;
    
    // Update turn rate with drift mechanics
    float turn_rate = AIRCRAFT_TURN_RATE;
    aircraft->drift_factor = 1.0f;
    aircraft->drifting = false;
    
    // Cool drift turns! (exactly as working version)
    if (stick_x < -0.1f && drift_right) {  // Turning left + R1 = drift
        turn_rate *= AIRCRAFT_DRIFT_TURN_MULTIPLIER;
        aircraft->drift_factor = AIRCRAFT_DRIFT_SLIDE_FACTOR;
        aircraft->drifting = true;
    } else if (stick_x > 0.1f && drift_left) {  // Turning right + L1 = drift
        turn_rate *= AIRCRAFT_DRIFT_TURN_MULTIPLIER;
        aircraft->drift_factor = AIRCRAFT_DRIFT_SLIDE_FACTOR;
        aircraft->drifting = true;
    }
    
    // Apply turning
    aircraft->yaw += stick_x * turn_rate * dt;
    
    // Banking (more extreme when drifting)
    float bank_angle = aircraft->drifting ? 60.0f : 35.0f;
    float target_roll = -stick_x * bank_angle;
    aircraft->roll = Lerp(aircraft->roll, target_roll, 5.0f * dt);
    
    // Pitching
    float target_pitch = stick_y * 40.0f;
    aircraft->pitch = Lerp(aircraft->pitch, target_pitch, 8.0f * dt);
    
    // Speed control
    if (brake) {
        aircraft->speed -= 60.0f * dt;
    } else if (!boost) {
        // Normal speed changes (not when boosting)
        if (aircraft->pitch < 0) {
            aircraft->speed += (-aircraft->pitch / 30.0f) * 30.0f * dt;
        } else {
            aircraft->speed -= (aircraft->pitch / 30.0f) * 5.0f * dt;
        }
    }
    
    aircraft->speed = Clamp(aircraft->speed, AIRCRAFT_MIN_SPEED, AIRCRAFT_MAX_SPEED);
    
    // Update actual speed with boost
    aircraft->actual_speed = aircraft->speed;
    if (boost) {
        aircraft->actual_speed *= AIRCRAFT_BOOST_MULTIPLIER;
    }
    
    // Update weapon cooldowns
    if (aircraft->gun_cooldown > 0) aircraft->gun_cooldown -= dt;
    if (aircraft->missile_cooldown > 0) aircraft->missile_cooldown -= dt;
    
    // Update stats
    aircraft->flight_time += dt;
}

void aircraft_update_physics(aircraft_t* aircraft, float dt) {
    // Get forward direction
    Vector3 forward = aircraft_get_forward_vector(aircraft);
    
    // Apply movement with drift (exactly as working version)
    if (aircraft->drift_factor < 1.0f) {
        // Keep some forward momentum while turning
        aircraft->drift_velocity = Vector3Lerp(aircraft->drift_velocity, forward, 3.0f * dt);
        aircraft->position = Vector3Add(aircraft->position, 
                                      Vector3Scale(aircraft->drift_velocity, 
                                                 aircraft->actual_speed * dt * aircraft->drift_factor));
        aircraft->position = Vector3Add(aircraft->position,
                                      Vector3Scale(forward, 
                                                 aircraft->actual_speed * dt * (1.0f - aircraft->drift_factor)));
    } else {
        aircraft->position = Vector3Add(aircraft->position, 
                                      Vector3Scale(forward, aircraft->actual_speed * dt));
    }
    
    // Keep above ground
    if (aircraft->position.y < 20) aircraft->position.y = 20;
    
    // Update distance traveled
    aircraft->distance_traveled += aircraft->actual_speed * dt;
    
    // Update visual effects
    aircraft->engine_glow = 0.5f + (aircraft->actual_speed / AIRCRAFT_MAX_SPEED) * 0.5f;
    if (aircraft->boosting) aircraft->engine_glow = 1.5f;
    
    if (aircraft->damage_flash > 0) {
        aircraft->damage_flash -= dt * 3.0f;
    }
}

void aircraft_apply_damage(aircraft_t* aircraft, int damage) {
    aircraft->health -= damage;
    if (aircraft->health < 0) aircraft->health = 0;
    aircraft->damage_flash = 1.0f;
}

void aircraft_repair(aircraft_t* aircraft, int amount) {
    aircraft->health += amount;
    if (aircraft->health > aircraft->max_health) {
        aircraft->health = aircraft->max_health;
    }
}

bool aircraft_is_alive(const aircraft_t* aircraft) {
    return aircraft->health > 0;
}

bool aircraft_is_boosting(const aircraft_t* aircraft) {
    return aircraft->boosting;
}

bool aircraft_is_drifting(const aircraft_t* aircraft) {
    return aircraft->drifting;
}

float aircraft_get_speed_percentage(const aircraft_t* aircraft) {
    return aircraft->actual_speed / AIRCRAFT_MAX_SPEED;
}

Vector3 aircraft_get_forward_vector(const aircraft_t* aircraft) {
    float yaw_rad = aircraft->yaw * DEG2RAD;
    float pitch_rad = aircraft->pitch * DEG2RAD;
    
    return (Vector3){
        sinf(yaw_rad) * cosf(pitch_rad),
        -sinf(pitch_rad),
        cosf(yaw_rad) * cosf(pitch_rad)
    };
}

Vector3 aircraft_get_aim_position(const aircraft_t* aircraft, float distance) {
    Vector3 forward = aircraft_get_forward_vector(aircraft);
    Vector3 aim_offset = {aircraft->gun_aim_x, aircraft->gun_aim_y, 0};
    Vector3 aim_dir = Vector3Normalize(Vector3Add(forward, Vector3Scale(aim_offset, 0.1f)));
    return Vector3Add(aircraft->position, Vector3Scale(aim_dir, distance));
}

Matrix aircraft_get_transform_matrix(const aircraft_t* aircraft) {
    Matrix transform = MatrixIdentity();
    transform = MatrixMultiply(transform, MatrixRotateY(aircraft->yaw * DEG2RAD));
    transform = MatrixMultiply(transform, MatrixRotateX(aircraft->pitch * DEG2RAD));
    transform = MatrixMultiply(transform, MatrixRotateZ(aircraft->roll * DEG2RAD));
    transform = MatrixMultiply(transform, MatrixTranslate(aircraft->position.x, 
                                                         aircraft->position.y, 
                                                         aircraft->position.z));
    return transform;
}

BoundingBox aircraft_get_bounding_box(const aircraft_t* aircraft) {
    Vector3 size = {10, 3, 15};  // Aircraft dimensions
    return (BoundingBox){
        Vector3Subtract(aircraft->position, Vector3Scale(size, 0.5f)),
        Vector3Add(aircraft->position, Vector3Scale(size, 0.5f))
    };
}
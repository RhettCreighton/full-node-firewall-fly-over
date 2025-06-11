/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/models/aircraft.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <math.h>

aircraft_t* aircraft_create(Vector3 start_pos) {
    aircraft_t* aircraft = calloc(1, sizeof(aircraft_t));
    if (!aircraft) return NULL;
    
    aircraft->position = start_pos;
    aircraft->speed = AIRCRAFT_BASE_SPEED;
    aircraft->yaw = 0;
    aircraft->pitch = 0;
    aircraft->roll = 0;
    aircraft->altitude = start_pos.y;
    aircraft->score = 0;
    aircraft->rings = 0;
    aircraft->fireTimer = 0;
    aircraft->missileTimer = 0;
    aircraft->missileSide = -1;  // Start with left missile
    
    return aircraft;
}

void aircraft_destroy(aircraft_t* aircraft) {
    if (!aircraft) return;
    free(aircraft);
}

void aircraft_update(aircraft_t* aircraft, float inputX, float inputY, float dt) {
    if (!aircraft) return;
    // Turning (yaw)
    aircraft->yaw += inputX * AIRCRAFT_TURN_RATE * dt;
    
    // Banking when turning (visual effect)
    float targetRoll = -inputX * 35.0f;
    aircraft->roll = Lerp(aircraft->roll, targetRoll, 5.0f * dt);
    
    // Pitching - ARCADE STYLE (no stalling!)
    float targetPitch = inputY * 40.0f;  // Down = pitch up, Up = pitch down
    aircraft->pitch = Lerp(aircraft->pitch, targetPitch, 4.0f * dt);
    
    // Speed changes based on pitch
    if (aircraft->pitch < 0) {
        aircraft->speed += (-aircraft->pitch / 30.0f) * 30.0f * dt;  // Big speed boost when diving
    } else {
        aircraft->speed -= (aircraft->pitch / 30.0f) * 5.0f * dt;   // Minimal speed loss when climbing
    }
    
    // Clamp speed
    aircraft->speed = Clamp(aircraft->speed, AIRCRAFT_MIN_SPEED, AIRCRAFT_MAX_SPEED * 1.5f);
    
    // Calculate movement direction
    Vector3 forward = aircraft_get_forward_vector(aircraft);
    
    // Update position
    aircraft->position = Vector3Add(aircraft->position, Vector3Scale(forward, aircraft->speed * dt));
    
    // Altitude affects speed slightly (gravity assist)
    if (aircraft->pitch < -5) {
        aircraft->speed += AIRCRAFT_GRAVITY * 0.1f * dt;
    }
    
    // Keep reasonable altitude
    aircraft->altitude = aircraft->position.y;
    if (aircraft->altitude < 20) {
        aircraft->position.y = 20;
    }
    
    // Apply world wrapping
    aircraft_wrap_position(aircraft);
    
    // Update weapon timers
    aircraft->fireTimer -= dt;
    aircraft->missileTimer -= dt;
}

void aircraft_barrel_roll_left(aircraft_t* aircraft, float dt) {
    if (!aircraft) return;
    aircraft->roll = Lerp(aircraft->roll, -70.0f, 4.0f * dt);
    aircraft->yaw -= AIRCRAFT_TURN_RATE * 1.5f * dt;
    aircraft->pitch = Lerp(aircraft->pitch, -10.0f, 3.0f * dt);
    aircraft->speed += 10.0f * dt;
}

void aircraft_barrel_roll_right(aircraft_t* aircraft, float dt) {
    if (!aircraft) return;
    aircraft->roll = Lerp(aircraft->roll, 70.0f, 4.0f * dt);
    aircraft->yaw += AIRCRAFT_TURN_RATE * 1.5f * dt;
    aircraft->pitch = Lerp(aircraft->pitch, -10.0f, 3.0f * dt);
    aircraft->speed += 10.0f * dt;
}

void aircraft_boost(aircraft_t* aircraft, float dt) {
    if (!aircraft) return;
    aircraft->speed += 60.0f * dt;
    aircraft->speed = Clamp(aircraft->speed, AIRCRAFT_MIN_SPEED, AIRCRAFT_MAX_SPEED * 1.5f);
}

void aircraft_brake(aircraft_t* aircraft, float dt) {
    if (!aircraft) return;
    aircraft->speed -= 50.0f * dt;
    aircraft->speed = Clamp(aircraft->speed, AIRCRAFT_MIN_SPEED, AIRCRAFT_MAX_SPEED * 1.5f);
}

Vector3 aircraft_get_forward_vector(aircraft_t* aircraft) {
    if (!aircraft) return (Vector3){0, 0, 1};
    return (Vector3){
        sinf(aircraft->yaw * DEG2RAD),
        -sinf(aircraft->pitch * DEG2RAD),
        cosf(aircraft->yaw * DEG2RAD)
    };
}

float aircraft_get_speed_percent(aircraft_t* aircraft) {
    if (!aircraft) return 0.0f;
    float range = AIRCRAFT_MAX_SPEED - AIRCRAFT_MIN_SPEED;
    if (range <= 0.0f) return 0.0f;
    return (aircraft->speed - AIRCRAFT_MIN_SPEED) / range;
}

void aircraft_draw(aircraft_t* aircraft, float gameTime) {
    if (!aircraft) return;
    rlPushMatrix();
    rlTranslatef(aircraft->position.x, aircraft->position.y, aircraft->position.z);
    rlRotatef(aircraft->yaw, 0, 1, 0);
    rlRotatef(aircraft->pitch, 1, 0, 0);
    rlRotatef(aircraft->roll, 0, 0, 1);
    
    // Fuselage
    DrawCube((Vector3){0, 0, 0}, 3, 2, 12, WHITE);
    DrawCube((Vector3){0, 0, -2}, 2.5f, 1.8f, 8, LIGHTGRAY);
    
    // Wings
    DrawCube((Vector3){0, 0, 0}, 25, 0.5f, 4, BLUE);
    
    // Tail
    DrawCube((Vector3){0, 3, -5}, 0.5f, 6, 3, BLUE);
    DrawCube((Vector3){0, 0, -5}, 8, 0.5f, 3, BLUE);
    
    // Propeller
    rlPushMatrix();
    rlTranslatef(0, 0, 6);
    rlRotatef(gameTime * 2000, 0, 0, 1);
    DrawCube((Vector3){0, 0, 0}, 8, 0.3f, 0.5f, GRAY);
    rlPopMatrix();
    
    // Cockpit
    DrawSphere((Vector3){0, 1, 1}, 1.2f, Fade(SKYBLUE, 0.7f));
    
    rlPopMatrix();
}

void aircraft_wrap_position(aircraft_t* aircraft) {
    if (!aircraft) return;
    // Wrap X coordinate (east-west)
    if (aircraft->position.x > WORLD_HALF_SIZE) {
        aircraft->position.x -= WORLD_SIZE;
    } else if (aircraft->position.x < -WORLD_HALF_SIZE) {
        aircraft->position.x += WORLD_SIZE;
    }
    
    // Wrap Z coordinate (north-south)
    if (aircraft->position.z > WORLD_HALF_SIZE) {
        aircraft->position.z -= WORLD_SIZE;
    } else if (aircraft->position.z < -WORLD_HALF_SIZE) {
        aircraft->position.z += WORLD_SIZE;
    }
    
    // Clamp Y coordinate (altitude)
    aircraft->position.y = Clamp(aircraft->position.y, WORLD_MIN_HEIGHT, WORLD_MAX_HEIGHT);
}
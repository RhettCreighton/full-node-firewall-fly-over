/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include "sky_combat/core/secure_code_points.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Terminal colors */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define CLEAR   "\033[2J\033[H"

/* Physics constants */
#define GRAVITY 9.81f
#define AIR_DENSITY 1.225f
#define WING_AREA 30.0f
#define STALL_ANGLE 15.0f
#define MAX_THRUST 50000.0f

/* Aircraft state */
typedef struct {
    /* Position */
    float x, y, z;  /* meters */
    
    /* Velocity */
    float vx, vy, vz;  /* m/s */
    
    /* Orientation */
    float pitch;  /* degrees */
    float roll;   /* degrees */
    float yaw;    /* degrees */
    
    /* Flight dynamics */
    float angle_of_attack;  /* degrees */
    float airspeed;         /* m/s */
    float altitude;         /* meters */
    float thrust_percent;   /* 0-100 */
    float lift_coefficient;
    float drag_coefficient;
    
    /* Status */
    int stalling;
    int crashed;
    float g_force;
} Aircraft;

/* Global aircraft */
static Aircraft aircraft = {
    .x = 0, .y = 1000, .z = 0,  /* Start at 1000m altitude */
    .vx = 100, .vy = 0, .vz = 0,  /* 100 m/s forward */
    .pitch = 0, .roll = 0, .yaw = 0,
    .thrust_percent = 50,
    .altitude = 1000
};

/* Non-blocking keyboard input */
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    
    return 0;
}

/* Calculate lift coefficient based on angle of attack */
float calculate_lift_coefficient(float aoa) {
    SECURE_CODE_POINT(CALCULATE_LIFT_COEFFICIENT, "calculating CL");
    
    if (fabs(aoa) > STALL_ANGLE) {
        SECURE_CODE_POINT(STALL_DETECTED, "AOA exceeds stall angle");
        return 0.3f;  /* Dramatic lift loss */
    }
    
    /* Simplified lift curve */
    return 0.1f + 0.08f * aoa;  /* CL increases with AOA until stall */
}

/* Update aircraft physics */
void update_physics(float dt) {
    SECURE_CODE_POINT(PHYSICS_UPDATE_START, "physics frame");
    
    /* Calculate airspeed */
    aircraft.airspeed = sqrtf(aircraft.vx * aircraft.vx + 
                             aircraft.vy * aircraft.vy + 
                             aircraft.vz * aircraft.vz);
    
    /* Angle of attack (simplified) */
    if (aircraft.airspeed > 0.1f) {
        aircraft.angle_of_attack = aircraft.pitch - 
            (atan2f(aircraft.vy, aircraft.vx) * 180.0f / M_PI);
    }
    
    /* Lift calculation */
    aircraft.lift_coefficient = calculate_lift_coefficient(aircraft.angle_of_attack);
    float lift = 0.5f * AIR_DENSITY * aircraft.airspeed * aircraft.airspeed * 
                 WING_AREA * aircraft.lift_coefficient;
    
    SECURE_CODE_POINT(LIFT_CALCULATED, "lift force computed");
    
    /* Drag calculation */
    aircraft.drag_coefficient = 0.02f + 0.05f * aircraft.lift_coefficient * aircraft.lift_coefficient;
    float drag = 0.5f * AIR_DENSITY * aircraft.airspeed * aircraft.airspeed * 
                 WING_AREA * aircraft.drag_coefficient;
    
    /* Thrust */
    float thrust = aircraft.thrust_percent / 100.0f * MAX_THRUST;
    
    /* Forces in body frame */
    float thrust_x = thrust * cosf(aircraft.pitch * M_PI / 180.0f);
    float thrust_y = thrust * sinf(aircraft.pitch * M_PI / 180.0f);
    
    /* Total forces */
    float fx = thrust_x - drag * (aircraft.vx / aircraft.airspeed);
    float fy = thrust_y + lift - GRAVITY * 5000;  /* 5000kg mass */
    float fz = 0;  /* Simplified - no lateral forces */
    
    SECURE_CODE_POINT(FORCES_CALCULATED, "all forces computed");
    
    /* Update velocity */
    aircraft.vx += fx / 5000 * dt;
    aircraft.vy += fy / 5000 * dt;
    aircraft.vz += fz / 5000 * dt;
    
    SECURE_CODE_POINT(VELOCITY_UPDATED, "velocity integrated");
    
    /* Update position */
    aircraft.x += aircraft.vx * dt;
    aircraft.y += aircraft.vy * dt;
    aircraft.z += aircraft.vz * dt;
    aircraft.altitude = aircraft.y;
    
    SECURE_CODE_POINT(POSITION_UPDATED, "position integrated");
    
    /* Check stall */
    aircraft.stalling = (fabs(aircraft.angle_of_attack) > STALL_ANGLE);
    if (aircraft.stalling) {
        SECURE_CODE_POINT(STALL_WARNING_ACTIVE, "STALL!");
        /* Add some turbulence */
        aircraft.pitch += (rand() % 3 - 1) * 0.5f;
        aircraft.roll += (rand() % 5 - 2) * 0.5f;
    }
    
    /* Ground collision */
    if (aircraft.altitude <= 0) {
        aircraft.altitude = 0;
        aircraft.crashed = 1;
        SECURE_CODE_POINT(GROUND_COLLISION, "crashed");
    }
    
    /* Calculate G-force */
    static float prev_vy = 0;
    aircraft.g_force = 1.0f + (aircraft.vy - prev_vy) / dt / GRAVITY;
    prev_vy = aircraft.vy;
    
    /* These must never happen */
    #ifdef PHYSICS_CHEATS_ENABLED
    SECURE_CODE_POINT(INFINITE_FUEL, "FORBIDDEN!");
    SECURE_CODE_POINT(PHYSICS_TELEPORT, "FORBIDDEN!");
    SECURE_CODE_POINT(IGNORE_STALL, "FORBIDDEN!");
    #endif
}

/* Draw aircraft attitude indicator */
void draw_attitude_indicator(void) {
    printf("\n");
    printf("     ATTITUDE      \n");
    printf("   ╔═══════════╗   \n");
    
    /* Horizon line position based on pitch */
    int horizon_pos = 3 - (int)(aircraft.pitch / 10.0f);
    if (horizon_pos < 0) horizon_pos = 0;
    if (horizon_pos > 6) horizon_pos = 6;
    
    for (int i = 0; i < 7; i++) {
        printf("   ║");
        
        if (i == horizon_pos) {
            printf("%s═══╪═══%s", GREEN, RESET);
        } else if (i < horizon_pos) {
            printf("%s   │   %s", CYAN, RESET);  /* Sky */
        } else {
            printf("%s   │   %s", YELLOW, RESET);  /* Ground */
        }
        
        printf("║");
        
        /* Roll indicator */
        if (i == 3) {
            int roll_pos = (int)(aircraft.roll / 10.0f);
            printf(" Roll: %+.1f°", aircraft.roll);
        }
        
        printf("\n");
    }
    
    printf("   ╚═══════════╝   \n");
}

/* Draw flight instruments */
void draw_instruments(void) {
    /* Airspeed indicator */
    printf("\n%sAIRSPEED%s          %sALTITUDE%s\n", CYAN, RESET, GREEN, RESET);
    printf("┌────────┐        ┌────────┐\n");
    printf("│%4.0f m/s│        │%5.0f m │\n", aircraft.airspeed, aircraft.altitude);
    printf("└────────┘        └────────┘\n");
    
    /* Angle of attack with stall warning */
    printf("\n%sANGLE OF ATTACK%s   ", aircraft.stalling ? RED : YELLOW, RESET);
    printf("%sTHRUST%s\n", MAGENTA, RESET);
    printf("┌────────┐        ┌────────┐\n");
    printf("│");
    
    if (aircraft.stalling) {
        printf("%sSTALL!%s  ", RED, RESET);
    } else {
        printf("%+4.1f°   ", aircraft.angle_of_attack);
    }
    
    printf("│        │%3.0f%%    │\n", aircraft.thrust_percent);
    printf("└────────┘        └────────┘\n");
    
    /* G-force meter */
    printf("\n%sG-FORCE%s: ", WHITE, RESET);
    int g_bars = (int)(aircraft.g_force * 5);
    for (int i = 0; i < 10; i++) {
        if (i < g_bars) {
            if (aircraft.g_force > 4) printf("%s█%s", RED, RESET);
            else if (aircraft.g_force > 2) printf("%s█%s", YELLOW, RESET);
            else printf("%s█%s", GREEN, RESET);
        } else {
            printf("░");
        }
    }
    printf(" %.1fG\n", aircraft.g_force);
}

/* Draw aircraft from side view */
void draw_aircraft_side(void) {
    printf("\n%sSIDE VIEW:%s\n", CYAN, RESET);
    
    /* Simple ASCII aircraft */
    if (aircraft.pitch > 20) {
        printf("      ╱\n");
        printf("     ╱\n");
        printf("  ══╱══\n");
        printf("   ╱\n");
    } else if (aircraft.pitch < -20) {
        printf("   ╲\n");
        printf("  ══╲══\n");
        printf("     ╲\n");
        printf("      ╲\n");
    } else {
        printf("    │\n");
        printf("  ══╪══\n");
        printf("    │\n");
    }
    
    /* Ground reference */
    int ground_distance = (int)(aircraft.altitude / 100);
    if (ground_distance > 10) ground_distance = 10;
    
    for (int i = 0; i < ground_distance; i++) {
        printf("\n");
    }
    
    if (!aircraft.crashed) {
        printf("%s════════════════%s\n", YELLOW, RESET);
    } else {
        printf("%s═══💥CRASHED💥═══%s\n", RED, RESET);
    }
}

/* Draw controls help */
void draw_controls(void) {
    printf("\n%sCONTROLS:%s\n", GREEN, RESET);
    printf("W/S - Pitch up/down    Q/E - Thrust -/+\n");
    printf("A/D - Roll left/right  SPACE - Center\n");
    printf("ESC - Exit\n");
}

/* Handle input */
void handle_input(void) {
    if (kbhit()) {
        char key = getchar();
        
        switch(key) {
            case 'w': case 'W':
                aircraft.pitch -= 2.0f;
                SECURE_CODE_POINT(PITCH_UP_INPUT, "stick back");
                break;
            case 's': case 'S':
                aircraft.pitch += 2.0f;
                SECURE_CODE_POINT(PITCH_DOWN_INPUT, "stick forward");
                break;
            case 'a': case 'A':
                aircraft.roll -= 5.0f;
                SECURE_CODE_POINT(ROLL_LEFT_INPUT, "stick left");
                break;
            case 'd': case 'D':
                aircraft.roll += 5.0f;
                SECURE_CODE_POINT(ROLL_RIGHT_INPUT, "stick right");
                break;
            case 'q': case 'Q':
                aircraft.thrust_percent -= 5.0f;
                if (aircraft.thrust_percent < 0) aircraft.thrust_percent = 0;
                SECURE_CODE_POINT(THRUST_DECREASE, "throttle down");
                break;
            case 'e': case 'E':
                aircraft.thrust_percent += 5.0f;
                if (aircraft.thrust_percent > 100) aircraft.thrust_percent = 100;
                SECURE_CODE_POINT(THRUST_INCREASE, "throttle up");
                break;
            case ' ':
                aircraft.pitch = 0;
                aircraft.roll = 0;
                SECURE_CODE_POINT(CENTER_CONTROLS, "center stick");
                break;
            case 27:  /* ESC */
                exit(0);
                break;
        }
        
        /* Limit angles */
        if (aircraft.pitch > 30) aircraft.pitch = 30;
        if (aircraft.pitch < -30) aircraft.pitch = -30;
        if (aircraft.roll > 45) aircraft.roll = 45;
        if (aircraft.roll < -45) aircraft.roll = -45;
    }
}

int main(void) {
    printf("%s", CLEAR);
    printf("%s=== FLIGHT TEST SCREEN ===%s\n", CYAN, RESET);
    printf("Testing aircraft physics with secure code points\n");
    
    /* Main loop */
    while (!aircraft.crashed) {
        /* Clear screen and redraw */
        printf("%s", CLEAR);
        printf("%s=== FLIGHT TEST SCREEN ===%s\n", CYAN, RESET);
        
        /* Update physics */
        update_physics(0.05f);  /* 50ms timestep */
        
        /* Draw everything */
        draw_attitude_indicator();
        draw_instruments();
        draw_aircraft_side();
        draw_controls();
        
        /* Handle input */
        handle_input();
        
        /* Small delay */
        usleep(50000);  /* 50ms = 20 FPS */
    }
    
    printf("\n%s*** CRASHED! ***%s\n", RED, RESET);
    printf("Press any key to exit...\n");
    getchar();
    
    return 0;
}
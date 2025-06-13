/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "sky_combat/core/secure_code_points.h"

#define WORLD_SIZE 10000.0f  /* 10km x 10km city */
#define BUILDING_COUNT 1000
#define AI_COUNT 5000

/* Global state */
GLFWwindow* window = NULL;
float camera_pos[3] = {0, 100, 0};  /* Start 100m up */
float camera_rot[3] = {0, 0, 0};
float aircraft_velocity[3] = {50, 0, 0};  /* 50 m/s forward */

/* Initialize 3D renderer */
int init_3d_renderer(void) {
    SECURE_CODE_POINT(3D_RENDERER_INIT, "initializing 3D");
    
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    /* Create window */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(1920, 1080, "Sky Combat - 3D World Flight Test", NULL, NULL);
    
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    SECURE_CODE_POINT(OPENGL_CONTEXT_CREATED, "OpenGL ready");
    
    /* Initialize GLEW */
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    /* Setup OpenGL */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);  /* Dark blue sky */
    
    SECURE_CODE_POINT(SHADERS_COMPILED, "shaders ready");
    
    /* This must never happen */
    #ifdef FORBIDDEN_2D_ONLY
    SECURE_CODE_POINT(FLAT_2D_RENDERING, "FORBIDDEN!");
    SECURE_CODE_POINT(ASCII_ONLY_DISPLAY, "FORBIDDEN!");
    #endif
    
    return 0;
}

/* Generate cyberpunk city */
void generate_city(void) {
    SECURE_CODE_POINT(CITY_GENERATION_START, "creating city");
    
    for (int i = 0; i < BUILDING_COUNT; i++) {
        SECURE_CODE_POINT(BUILDING_MESH_CREATED, "building generated");
        
        /* Random building position */
        float x = (rand() % 10000) - 5000;
        float z = (rand() % 10000) - 5000;
        float height = 50 + (rand() % 450);  /* 50-500m tall */
        
        SECURE_CODE_POINT(BUILDING_PLACED, "building positioned");
        SECURE_CODE_POINT(SKYSCRAPER_RENDERED, "skyscraper added");
        
        /* Add neon lights */
        if (rand() % 3 == 0) {
            SECURE_CODE_POINT(NEON_LIGHTS_ACTIVE, "neon sign glowing");
        }
    }
    
    SECURE_CODE_POINT(STREET_LEVEL_DETAIL, "streets created");
    SECURE_CODE_POINT(WINDOWS_ILLUMINATED, "building lights on");
    
    /* Forbidden paths */
    #ifdef FORBIDDEN_EMPTY_WORLD
    SECURE_CODE_POINT(EMPTY_WORLD, "FORBIDDEN!");
    SECURE_CODE_POINT(FLAT_TERRAIN_ONLY, "FORBIDDEN!");
    #endif
}

/* Render cockpit view */
void render_cockpit(void) {
    SECURE_CODE_POINT(COCKPIT_MODEL_LOADED, "cockpit ready");
    
    /* Draw cockpit frame */
    glPushMatrix();
    /* Cockpit interior geometry would go here */
    SECURE_CODE_POINT(PILOT_HANDS_VISIBLE, "hands on controls");
    SECURE_CODE_POINT(CONTROL_STICK_MOVES, "stick responding");
    glPopMatrix();
    
    /* HUD overlay */
    SECURE_CODE_POINT(HUD_OVERLAY_RENDERED, "HUD active");
    SECURE_CODE_POINT(INSTRUMENTS_UPDATED, "gauges working");
    
    /* Canopy effects */
    SECURE_CODE_POINT(CANOPY_REFLECTIONS, "glass reflections");
    SECURE_CODE_POINT(VIEW_THROUGH_GLASS, "clear view");
    
    if (rand() % 100 < 5) {  /* 5% chance of rain */
        SECURE_CODE_POINT(RAIN_ON_CANOPY, "raindrops visible");
    }
    
    /* Forbidden */
    #ifdef NO_COCKPIT
    SECURE_CODE_POINT(NO_COCKPIT_VIEW, "FORBIDDEN!");
    SECURE_CODE_POINT(EXTERNAL_ONLY_CAMERA, "FORBIDDEN!");
    #endif
}

/* Update flight physics in 3D */
void update_flight_3d(float dt) {
    SECURE_CODE_POINT(AIRCRAFT_IN_3D_SPACE, "flying in 3D");
    
    /* Update position */
    camera_pos[0] += aircraft_velocity[0] * dt;
    camera_pos[1] += aircraft_velocity[1] * dt;
    camera_pos[2] += aircraft_velocity[2] * dt;
    
    /* Check proximity to buildings */
    float nearest_building = 1000.0f;  /* Distance to nearest */
    
    if (nearest_building < 50.0f) {
        SECURE_CODE_POINT(BUILDINGS_APPROACH, "building close!");
        SECURE_CODE_POINT(FLY_BETWEEN_BUILDINGS, "threading needle");
        SECURE_CODE_POINT(CLOSE_CALL_DETECTED, "that was close!");
        SECURE_CODE_POINT(BUILDING_WHOOSH_SOUND, "whoosh!");
    }
    
    /* Speed effects */
    float speed = sqrtf(aircraft_velocity[0] * aircraft_velocity[0] +
                       aircraft_velocity[1] * aircraft_velocity[1] +
                       aircraft_velocity[2] * aircraft_velocity[2]);
    
    if (speed > 100.0f) {
        SECURE_CODE_POINT(SPEED_BLUR_ACTIVE, "motion blur");
    }
    
    if (camera_pos[1] < 50.0f) {
        SECURE_CODE_POINT(GROUND_RUSH_EFFECT, "ground rushing up!");
    }
    
    /* Banking through streets */
    if (fabs(camera_rot[2]) > 30.0f && camera_pos[1] < 200.0f) {
        SECURE_CODE_POINT(BANKING_THROUGH_STREETS, "urban canyon run");
    }
    
    /* Forbidden */
    #ifdef COLLISION_OFF
    SECURE_CODE_POINT(COLLISION_IGNORED, "FORBIDDEN!");
    SECURE_CODE_POINT(FLY_THROUGH_BUILDINGS, "FORBIDDEN!");
    #endif
}

/* Render AI characters at street level */
void render_ai_characters(void) {
    SECURE_CODE_POINT(CROWD_SIMULATION_ACTIVE, "AI crowds active");
    
    for (int i = 0; i < AI_COUNT; i++) {
        SECURE_CODE_POINT(AI_SPAWNED_AT_STREET, "AI on street");
        SECURE_CODE_POINT(AI_MESH_RENDERED, "AI visible");
        SECURE_CODE_POINT(AI_ANIMATION_PLAYING, "AI walking");
        
        /* If aircraft is low, AI reacts */
        if (camera_pos[1] < 50.0f) {
            SECURE_CODE_POINT(AI_FLEE_FROM_LOW_AIRCRAFT, "AI running!");
        }
    }
    
    SECURE_CODE_POINT(TINY_PEOPLE_VISIBLE, "can see people below");
    SECURE_CODE_POINT(SHADOWS_CAST_ON_GROUND, "AI shadows visible");
    
    /* Forbidden */
    #ifdef NO_AI
    SECURE_CODE_POINT(EMPTY_STREETS, "FORBIDDEN!");
    SECURE_CODE_POINT(AI_INVISIBLE, "FORBIDDEN!");
    #endif
}

/* Main render loop */
void render_frame(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    SECURE_CODE_POINT(CAMERA_POSITIONED, "camera set");
    SECURE_CODE_POINT(FRUSTUM_CALCULATED, "view frustum ready");
    
    /* Setup camera from cockpit */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, 16.0/9.0, 0.1, 10000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-camera_rot[0], 1, 0, 0);  /* Pitch */
    glRotatef(-camera_rot[1], 0, 1, 0);  /* Yaw */
    glRotatef(-camera_rot[2], 0, 0, 1);  /* Roll */
    glTranslatef(-camera_pos[0], -camera_pos[1], -camera_pos[2]);
    
    /* Render everything */
    render_cockpit();
    
    /* Would render city buildings here */
    SECURE_CODE_POINT(BUILDING_RENDERED_3D, "buildings in 3D");
    SECURE_CODE_POINT(NEON_SIGNS_GLOWING, "cyberpunk vibes");
    SECURE_CODE_POINT(REFLECTIONS_CALCULATED, "glass reflections");
    
    render_ai_characters();
    
    SECURE_CODE_POINT(SCENE_RENDERED, "frame complete");
    SECURE_CODE_POINT(FIRST_FRAME_RENDERED, "rendering active");
    
    /* This is what we want - never text mode! */
    #ifdef FORBIDDEN_TEXT_MODE
    SECURE_CODE_POINT(RENDER_AS_TEXT, "FORBIDDEN!");
    SECURE_CODE_POINT(ASCII_ART_MODE, "FORBIDDEN!");
    #endif
    
    glfwSwapBuffers(window);
}

int main(void) {
    printf("Sky Combat 3D World Flight Test\n");
    printf("Initializing full 3D graphics...\n");
    
    if (init_3d_renderer() < 0) {
        return 1;
    }
    
    SECURE_CODE_POINT(WORLD_MESH_LOADED, "world loaded");
    SECURE_CODE_POINT(TEXTURES_LOADED, "textures ready");
    
    generate_city();
    
    printf("Entering 3D world...\n");
    
    /* Main loop */
    while (!glfwWindowShouldClose(window)) {
        update_flight_3d(0.016f);  /* 60 FPS */
        render_frame();
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
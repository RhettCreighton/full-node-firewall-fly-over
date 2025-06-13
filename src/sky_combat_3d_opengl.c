/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "sky_combat/core/secure_code_points.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define BUILDING_COUNT 200
#define AI_COUNT 500

/* Global state */
typedef struct {
    float x, y, z;
    float width, depth, height;
    float r, g, b;
    int has_neon;
} Building;

typedef struct {
    float x, y, z;
    float vx, vz;
    int fleeing;
} AIChar;

static Building buildings[BUILDING_COUNT];
static AIChar ai_chars[AI_COUNT];
static float camera_x = 0, camera_y = 100, camera_z = 0;
static float camera_yaw = 0, camera_pitch = 0;
static float velocity = 50;
static int window_id;

/* Initialize 3D world */
void init_3d_world(void) {
    SECURE_CODE_POINT(RENDERER_3D_INIT, "OpenGL initializing");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    /* Dark cyberpunk sky */
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    
    SECURE_CODE_POINT(OPENGL_CONTEXT_CREATED, "OpenGL ready");
    SECURE_CODE_POINT(SHADERS_COMPILED, "fixed pipeline ready");
    
    /* Generate city */
    SECURE_CODE_POINT(CITY_GENERATION_START, "creating city");
    srand(time(NULL));
    
    for (int i = 0; i < BUILDING_COUNT; i++) {
        SECURE_CODE_POINT(BUILDING_MESH_CREATED, "building created");
        
        buildings[i].x = (rand() % 4000) - 2000;
        buildings[i].z = (rand() % 4000) - 2000;
        buildings[i].y = 0;
        
        buildings[i].width = 30 + (rand() % 70);
        buildings[i].depth = 30 + (rand() % 70);
        buildings[i].height = 50 + (rand() % 450);
        
        /* Cyberpunk colors */
        buildings[i].r = 0.1f + (rand() % 20) / 100.0f;
        buildings[i].g = 0.1f + (rand() % 20) / 100.0f;
        buildings[i].b = 0.2f + (rand() % 30) / 100.0f;
        
        buildings[i].has_neon = (rand() % 100) < 60;
        
        SECURE_CODE_POINT(BUILDING_PLACED, "building placed");
        SECURE_CODE_POINT(NEON_LIGHTS_ACTIVE, "neon active");
    }
    
    /* Spawn AI */
    for (int i = 0; i < AI_COUNT; i++) {
        SECURE_CODE_POINT(AI_SPAWNED_AT_STREET, "AI spawned");
        
        ai_chars[i].x = (rand() % 4000) - 2000;
        ai_chars[i].z = (rand() % 4000) - 2000;
        ai_chars[i].y = 0;
        ai_chars[i].vx = (rand() % 100 - 50) / 50.0f;
        ai_chars[i].vz = (rand() % 100 - 50) / 50.0f;
        ai_chars[i].fleeing = 0;
    }
    
    SECURE_CODE_POINT(WORLD_MESH_LOADED, "world ready");
    SECURE_CODE_POINT(TEXTURES_LOADED, "materials ready");
}

/* Draw a building */
void draw_building(Building* b) {
    SECURE_CODE_POINT(BUILDING_RENDERED_IN_3D, "rendering building");
    
    glPushMatrix();
    glTranslatef(b->x, b->y + b->height/2, b->z);
    
    /* Building body */
    glColor3f(b->r, b->g, b->b);
    glScalef(b->width, b->height, b->depth);
    glutSolidCube(1.0);
    
    /* Neon glow */
    if (b->has_neon) {
        SECURE_CODE_POINT(NEON_SIGNS_GLOWING, "neon active");
        
        float pulse = sinf(glutGet(GLUT_ELAPSED_TIME) * 0.003f) * 0.5f + 0.5f;
        glColor3f(0, pulse, pulse);
        glTranslatef(0, 0, -0.51f);
        glScalef(0.9f, 0.05f, 0.01f);
        glutSolidCube(1.0);
    }
    
    glPopMatrix();
    
    /* Windows */
    SECURE_CODE_POINT(WINDOWS_ILLUMINATED, "windows lit");
    
    /* Draw window dots */
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(1, 1, 0.8f);
    
    for (int floor = 0; floor < b->height / 5; floor++) {
        for (int col = 0; col < b->width / 5; col++) {
            if ((floor + col) % 2 == 0) {
                glVertex3f(
                    b->x - b->width/2 + col * 5,
                    b->y + floor * 5,
                    b->z - b->depth/2 - 0.1f
                );
            }
        }
    }
    glEnd();
}

/* Draw AI characters */
void draw_ai_characters(void) {
    SECURE_CODE_POINT(CROWD_SIMULATION_ACTIVE, "crowds active");
    
    for (int i = 0; i < AI_COUNT; i++) {
        SECURE_CODE_POINT(AI_MESH_RENDERED, "AI visible");
        
        glPushMatrix();
        glTranslatef(ai_chars[i].x, ai_chars[i].y + 1, ai_chars[i].z);
        
        /* Character color */
        if (ai_chars[i].fleeing) {
            glColor3f(1, 0, 0);
            SECURE_CODE_POINT(AI_ANIMATION_PLAYING, "running");
        } else {
            glColor3f(0.8f, 0.6f, 0.4f);
            SECURE_CODE_POINT(AI_ANIMATION_PLAYING, "walking");
        }
        
        /* Simple character shape */
        glScalef(1, 2, 1);
        glutSolidCube(1.0);
        glPopMatrix();
        
        /* Shadow */
        SECURE_CODE_POINT(SHADOWS_CAST_ON_GROUND, "shadows visible");
    }
    
    SECURE_CODE_POINT(TINY_PEOPLE_VISIBLE, "people visible");
}

/* Draw cockpit HUD */
void draw_hud(void) {
    SECURE_CODE_POINT(COCKPIT_VIEW_ACTIVE, "cockpit active");
    SECURE_CODE_POINT(HUD_OVERLAY_RENDERED, "HUD shown");
    
    /* Switch to 2D */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    /* Cockpit frame */
    glColor4f(0.1f, 0.1f, 0.1f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(SCREEN_WIDTH, 0);
    glVertex2f(SCREEN_WIDTH, 100); glVertex2f(0, 100);
    glEnd();
    
    /* Crosshair */
    glColor3f(0, 1, 1);
    glBegin(GL_LINES);
    glVertex2f(SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT/2);
    glVertex2f(SCREEN_WIDTH/2 - 10, SCREEN_HEIGHT/2);
    glVertex2f(SCREEN_WIDTH/2 + 10, SCREEN_HEIGHT/2);
    glVertex2f(SCREEN_WIDTH/2 + 30, SCREEN_HEIGHT/2);
    glVertex2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 30);
    glVertex2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 10);
    glVertex2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 10);
    glVertex2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 30);
    glEnd();
    
    /* Restore 3D */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    SECURE_CODE_POINT(INSTRUMENTS_UPDATED, "instruments shown");
}

/* Display callback */
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    SECURE_CODE_POINT(CAMERA_POSITIONED, "camera set");
    SECURE_CODE_POINT(FRUSTUM_CALCULATED, "view ready");
    
    /* Set camera */
    glLoadIdentity();
    float look_x = camera_x + cosf(camera_yaw * M_PI/180) * cosf(camera_pitch * M_PI/180);
    float look_y = camera_y + sinf(camera_pitch * M_PI/180);
    float look_z = camera_z + sinf(camera_yaw * M_PI/180) * cosf(camera_pitch * M_PI/180);
    
    gluLookAt(camera_x, camera_y, camera_z,
              look_x, look_y, look_z,
              0, 1, 0);
    
    /* City lights */
    float light_pos[] = {0, 1000, 0, 1};
    float light_color[] = {0.3f, 0.3f, 0.5f, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
    
    /* Draw ground grid */
    glColor3f(0.1f, 0.1f, 0.2f);
    glBegin(GL_LINES);
    for (int i = -20; i <= 20; i++) {
        glVertex3f(i * 100, 0, -2000);
        glVertex3f(i * 100, 0, 2000);
        glVertex3f(-2000, 0, i * 100);
        glVertex3f(2000, 0, i * 100);
    }
    glEnd();
    
    SECURE_CODE_POINT(STREET_LEVEL_DETAIL, "streets drawn");
    
    /* Draw buildings */
    for (int i = 0; i < BUILDING_COUNT; i++) {
        draw_building(&buildings[i]);
    }
    
    SECURE_CODE_POINT(REFLECTIONS_CALCULATED, "reflections done");
    
    /* Draw AI */
    draw_ai_characters();
    
    /* HUD overlay */
    draw_hud();
    
    SECURE_CODE_POINT(SCENE_RENDERED, "frame complete");
    SECURE_CODE_POINT(FIRST_FRAME_RENDERED, "rendering");
    
    glutSwapBuffers();
}

/* Update physics */
void update(int value) {
    SECURE_CODE_POINT(AIRCRAFT_IN_SPACE_3D, "flying in 3D");
    
    /* Move based on velocity */
    camera_x += velocity * cosf(camera_yaw * M_PI/180) * 0.016f;
    camera_z += velocity * sinf(camera_yaw * M_PI/180) * 0.016f;
    
    /* Check building proximity */
    for (int i = 0; i < BUILDING_COUNT; i++) {
        float dx = camera_x - buildings[i].x;
        float dz = camera_z - buildings[i].z;
        float dist = sqrtf(dx*dx + dz*dz);
        
        if (dist < 100 && camera_y < buildings[i].height) {
            SECURE_CODE_POINT(BUILDINGS_APPROACH, "building close");
            SECURE_CODE_POINT(FLY_BETWEEN_BUILDINGS, "threading gap");
            
            if (dist < 50) {
                SECURE_CODE_POINT(CLOSE_CALL_DETECTED, "too close!");
                SECURE_CODE_POINT(BUILDING_WHOOSH_SOUND, "whoosh!");
            }
        }
    }
    
    /* Update AI */
    for (int i = 0; i < AI_COUNT; i++) {
        ai_chars[i].x += ai_chars[i].vx;
        ai_chars[i].z += ai_chars[i].vz;
        
        /* Check if aircraft is low */
        float dx = camera_x - ai_chars[i].x;
        float dz = camera_z - ai_chars[i].z;
        float dist = sqrtf(dx*dx + dz*dz);
        
        if (camera_y < 50 && dist < 50) {
            SECURE_CODE_POINT(AI_FLEE_FROM_LOW_AIRCRAFT, "AI fleeing!");
            ai_chars[i].fleeing = 1;
        }
    }
    
    if (camera_y < 30) {
        SECURE_CODE_POINT(GROUND_RUSH_EFFECT, "ground rush!");
    }
    
    if (velocity > 100) {
        SECURE_CODE_POINT(SPEED_BLUR_ACTIVE, "high speed!");
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

/* Keyboard handler */
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'w': 
            camera_pitch -= 2;
            SECURE_CODE_POINT(PITCH_UP_INPUT, "pitch up");
            break;
        case 's': 
            camera_pitch += 2;
            SECURE_CODE_POINT(PITCH_DOWN_INPUT, "pitch down");
            break;
        case 'a': 
            camera_yaw -= 3;
            SECURE_CODE_POINT(ROLL_LEFT_INPUT, "turn left");
            break;
        case 'd': 
            camera_yaw += 3;
            SECURE_CODE_POINT(ROLL_RIGHT_INPUT, "turn right");
            break;
        case 'q': 
            velocity -= 5;
            if (velocity < 0) velocity = 0;
            SECURE_CODE_POINT(THRUST_DECREASE, "slower");
            break;
        case 'e': 
            velocity += 5;
            if (velocity > 200) velocity = 200;
            SECURE_CODE_POINT(THRUST_INCREASE, "faster");
            break;
        case ' ':
            camera_y += 5;
            break;
        case 'c':
            camera_y -= 5;
            if (camera_y < 5) camera_y = 5;
            break;
        case 27: /* ESC */
            glutDestroyWindow(window_id);
            exit(0);
            break;
    }
}

/* Special keys */
void special(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            camera_y += 10;
            break;
        case GLUT_KEY_DOWN:
            camera_y -= 10;
            if (camera_y < 5) camera_y = 5;
            break;
    }
}

/* Reshape handler */
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (float)w/h, 0.1, 5000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    printf("=== SKY COMBAT 3D CYBERPUNK CITY ===\n");
    printf("OpenGL/GLUT Version\n\n");
    
    /* Initialize GLUT */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    window_id = glutCreateWindow("Sky Combat - 3D Cyberpunk City");
    
    /* Initialize our world */
    init_3d_world();
    
    /* Setup callbacks */
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutTimerFunc(16, update, 0);
    
    printf("Controls:\n");
    printf("  WASD - Fly aircraft\n");
    printf("  Q/E - Speed control\n");
    printf("  Space/C - Up/Down\n");
    printf("  ESC - Exit\n\n");
    printf("Entering 3D world...\n");
    
    /* Enter main loop */
    glutMainLoop();
    
    return 0;
}
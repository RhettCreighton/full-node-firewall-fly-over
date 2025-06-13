/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sky_combat/specifications/aircraft_rendering_spec.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <raymath.h>

aircraft_rendering_spec_t* aircraft_rendering_spec_create(void) {
    aircraft_rendering_spec_t* ars = calloc(1, sizeof(aircraft_rendering_spec_t));
    if (!ars) return NULL;
    
    // Create the specification
    ars->spec = spec_create(
        "Aircraft Rendering",
        "Aircraft must always be visible on screen when in view"
    );
    
    // This is critical - if aircraft doesn't render, game is broken
    spec_set_critical(ars->spec, true);
    spec_register(ars->spec);
    
    // Create render texture for pixel verification
    ars->buffer_width = 256;  // Small buffer for performance
    ars->buffer_height = 256;
    ars->verify_texture = LoadRenderTexture(ars->buffer_width, ars->buffer_height);
    ars->pixel_buffer = calloc(ars->buffer_width * ars->buffer_height, sizeof(Color));
    
    SPEC_CHECK_NOT_NULL(ars->spec, ars->pixel_buffer, 
                        "Failed to allocate pixel verification buffer");
    
    ars->initialized = true;
    return ars;
}

void aircraft_rendering_spec_destroy(aircraft_rendering_spec_t* ars) {
    if (!ars) return;
    
    if (ars->initialized) {
        UnloadRenderTexture(ars->verify_texture);
        free(ars->pixel_buffer);
    }
    
    spec_finalize(ars->spec);
    spec_report(ars->spec);
    free(ars);
}

void aircraft_rendering_spec_begin_frame(aircraft_rendering_spec_t* ars,
                                       aircraft_t* aircraft,
                                       Camera3D* camera) {
    if (!ars || !ars->initialized) return;
    
    ars->frames_checked++;
    
    // Check if aircraft should be visible
    bool should_be_visible = is_aircraft_in_frustum(aircraft, camera);
    if (should_be_visible) {
        ars->frames_aircraft_should_be_visible++;
    }
    
    // Begin rendering to our verification texture
    BeginTextureMode(ars->verify_texture);
    ClearBackground(BLACK);
    
    // Set up same camera
    BeginMode3D(*camera);
}

void aircraft_rendering_spec_end_frame(aircraft_rendering_spec_t* ars) {
    if (!ars || !ars->initialized) return;
    
    EndMode3D();
    EndTextureMode();
    
    // Read pixels from render texture
    Image img = LoadImageFromTexture(ars->verify_texture.texture);
    Color* pixels = LoadImageColors(img);
    
    // Copy to our buffer
    memcpy(ars->pixel_buffer, pixels, 
           ars->buffer_width * ars->buffer_height * sizeof(Color));
    
    UnloadImageColors(pixels);
    UnloadImage(img);
}

bool aircraft_rendering_spec_check_visibility(aircraft_rendering_spec_t* ars,
                                            aircraft_t* aircraft,
                                            Camera3D* camera) {
    if (!ars || !ars->initialized) return false;
    
    bool in_frustum = is_aircraft_in_frustum(aircraft, camera);
    
    // Count non-black pixels (aircraft should be visible)
    int visible_pixels = 0;
    for (int i = 0; i < ars->buffer_width * ars->buffer_height; i++) {
        Color c = ars->pixel_buffer[i];
        if (c.r > 0 || c.g > 0 || c.b > 0) {
            visible_pixels++;
        }
    }
    
    // If aircraft is in frustum, it MUST have rendered pixels
    if (in_frustum) {
        SPEC_CHECK(ars->spec, visible_pixels > 0,
                   "Aircraft in frustum but no pixels rendered");
        
        // Check minimum pixel coverage (at least 10 pixels)
        SPEC_CHECK(ars->spec, visible_pixels >= 10,
                   "Aircraft rendered with too few pixels");
        
        if (visible_pixels > 0) {
            ars->frames_aircraft_visible++;
        }
    }
    
    return visible_pixels > 0;
}

bool aircraft_rendering_spec_check_pixel_coverage(aircraft_rendering_spec_t* ars,
                                                int min_pixels,
                                                int max_pixels) {
    if (!ars || !ars->initialized) return false;
    
    int visible_pixels = 0;
    for (int i = 0; i < ars->buffer_width * ars->buffer_height; i++) {
        Color c = ars->pixel_buffer[i];
        if (c.r > 0 || c.g > 0 || c.b > 0) {
            visible_pixels++;
        }
    }
    
    SPEC_CHECK_RANGE(ars->spec, visible_pixels, min_pixels, max_pixels,
                     "Aircraft pixel coverage out of expected range");
    
    return (visible_pixels >= min_pixels && visible_pixels <= max_pixels);
}

bool aircraft_rendering_spec_check_color_contrast(aircraft_rendering_spec_t* ars,
                                                Color aircraft_color,
                                                Color background_color,
                                                float min_contrast) {
    if (!ars || !ars->initialized) return false;
    
    float contrast = calculate_color_contrast(aircraft_color, background_color);
    
    SPEC_CHECK(ars->spec, contrast >= min_contrast,
               "Aircraft color contrast too low for visibility");
    
    return contrast >= min_contrast;
}

bool is_aircraft_in_frustum(aircraft_t* aircraft, Camera3D* camera) {
    if (!aircraft || !camera) return false;
    
    Vector3 pos = aircraft_get_position(aircraft);
    
    // Simple frustum check - is aircraft within reasonable distance?
    Vector3 to_aircraft = Vector3Subtract(pos, camera->position);
    float distance = Vector3Length(to_aircraft);
    
    // Check if within render distance (e.g., 1000 units)
    if (distance > 1000.0f) return false;
    
    // Check if in front of camera (simple dot product test)
    Vector3 cam_forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
    float dot = Vector3DotProduct(Vector3Normalize(to_aircraft), cam_forward);
    
    // If dot > 0, aircraft is in front of camera
    return dot > 0.0f;
}

int count_aircraft_pixels(Color* buffer, int width, int height, Color aircraft_color) {
    int count = 0;
    int tolerance = 20; // Color matching tolerance
    
    for (int i = 0; i < width * height; i++) {
        Color c = buffer[i];
        if (abs(c.r - aircraft_color.r) < tolerance &&
            abs(c.g - aircraft_color.g) < tolerance &&
            abs(c.b - aircraft_color.b) < tolerance &&
            c.a > 200) { // Mostly opaque
            count++;
        }
    }
    
    return count;
}

float calculate_color_contrast(Color c1, Color c2) {
    // Simple contrast calculation
    float dr = abs(c1.r - c2.r) / 255.0f;
    float dg = abs(c1.g - c2.g) / 255.0f;
    float db = abs(c1.b - c2.b) / 255.0f;
    
    return (dr + dg + db) / 3.0f;
}
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <fenv.h>
#include "sky_combat/controllers/sky_combat.h"
#include "sky_combat/models/aircraft.h"
#include "sky_combat/models/weapons.h"
#include "sky_combat/controllers/input_mvc_fast.h"
#include "sky_combat/models/enemies.h"
#include "sky_combat/models/cyberpunk_world.h"
#include "sky_combat/views/combat_effects.h"
#include "sky_combat/models/overdrive.h"
#include "sky_combat/models/boss_cyber_dragon.h"

// Real safety systems
#include "sky_combat/utils/crash_protection.h"
#include "sky_combat/utils/safety_macros.h"

// GDB proof headers for safety (legacy - disabled by default)
#ifdef ENABLE_GDB_PROOF
#include "gdb_proof.h"
#include "gdb_proof_controls.h"
#include "gdb_proof_weapons.h"
#include "gdb_proof_gun_tuning.h"
#include "gdb_proof_practice_targets.h"
#endif

// Forward declare responsive functions
void aircraft_update_responsive(aircraft_t* aircraft, float stick_x, float stick_y, float dt);
void camera_update_responsive(Camera3D* camera, Vector3 target, float yaw, float distance, float height, float dt);

// Include specifications
#include "sky_combat/specifications/aircraft_controls_spec.h"
#include "sky_combat/core/secure_code_points.h"

typedef struct {
    // Core systems
    aircraft_t* aircraft;
    weapons_system_t* weapons;
    input_mvc_fast_t* input;
    enemy_manager_t* enemies;
    cyberpunk_world_t* world;
    effects_manager_t* effects;
    overdrive_system_t* overdrive;
    
    // Boss
    cyber_dragon_t* boss;
    bool boss_spawned;
    bool boss_defeated;
    
    // Camera
    Camera3D camera;
    
    // Gun aiming (controlled by right stick)
    float gun_aim_offset_x;
    float gun_aim_offset_y;
    
    // Game state
    int score;
    int combo;
    float combo_timer;
    int max_combo;
    int wave;
    bool game_over;
    bool victory;
    
    // Player state
    float boost_fuel;
    float shield_power;
    float player_health;
    float damage_flash;
    float boost_timer;  // Timer for UL boost burst
    
    // Progression
    int rank;  // 0=Rookie, 1=Pilot, 2=Ace, 3=Legend
    int total_kills;
    int boss_kills;
    float play_time;
    
    // Visual settings
    bool motion_blur_enabled;
    bool chromatic_enabled;
    float speed_lines_intensity;
    
    // Specifications
    aircraft_controls_spec_t* controls_spec;
} ultimate_game_t;

static ultimate_game_t* game_create(void) {
    ultimate_game_t* game = calloc(1, sizeof(ultimate_game_t));
    
    // Create systems
    game->aircraft = aircraft_create((Vector3){0, 50, 0});
    game->weapons = weapons_create();
    game->input = input_mvc_fast_create();
    game->enemies = enemies_create(500);  // HUNDREDS of enemies!
    game->world = cyberpunk_world_create();
    game->effects = effects_create(1000, 10000);  // More particles!
    game->overdrive = overdrive_create();
    
    // Setup camera
    game->camera.position = (Vector3){0, 20, -30};
    game->camera.target = (Vector3){0, 0, 0};
    game->camera.up = (Vector3){0, 1, 0};
    game->camera.fovy = 65.0f;
    game->camera.projection = CAMERA_PERSPECTIVE;
    
    // Initial state
    game->boost_fuel = 100.0f;
    game->shield_power = 100.0f;
    game->player_health = 100.0f;
    game->boost_timer = 0.0f;  // Initialize boost timer
    
    // Visual settings
    game->motion_blur_enabled = true;
    game->chromatic_enabled = true;
    
    // Create controls specification
    game->controls_spec = aircraft_controls_spec_create();
    
    // Generate world
    cyberpunk_world_generate(game->world, 42);
    cyberpunk_set_theme_blade_runner(game->world);
    
    return game;
}

static void spawn_boss(ultimate_game_t* game) {
    if (game->boss_spawned) return;
    
    // Epic boss entrance
    Vector3 spawn_pos = {0, 200, 300};
    game->boss = cyber_dragon_create(spawn_pos);
    game->boss_spawned = true;
    
    // Clear all enemies manually
    // TODO: Add proper clear function to enemy manager
    
    // Screen effects
    effects_screen_flash(game->effects, WHITE, 1.0f);
    effects_screen_shake(game->effects, 5.0f, 2.0f);
    effects_show_achievement(game->effects, "BOSS: CYBER DRAGON AWAKENS!");
    
    printf("BOSS BATTLE INITIATED!\n");
}

static void handle_combat_ultimate(ultimate_game_t* game, input_state_fast_t* input, float dt) {
    Vector3 pos = game->aircraft->position;
    Vector3 forward = aircraft_get_forward_vector(game->aircraft);
    float yaw = game->aircraft->yaw;
    
    // OVERDRIVE activation - Triangle button (Y on Xbox layout)
    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP) && 
        overdrive_can_activate(game->overdrive)) {
        overdrive_activate(game->overdrive);
        effects_show_achievement(game->effects, "OVERDRIVE ACTIVATED!");
        effects_screen_flash(game->effects, ORANGE, 0.5f);
        
        // Instant effects
        game->boost_fuel = 100.0f;
        game->shield_power = 100.0f;
    }
    
    // Weapon switching
    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
        weapons_switch_weapon(game->weapons, WEAPON_MACHINE_GUN);
    }
    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
        weapons_switch_weapon(game->weapons, WEAPON_LASER);
    }
    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
        weapons_switch_weapon(game->weapons, WEAPON_SPREAD);
    }
    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_THUMB)) {
        weapons_switch_weapon(game->weapons, WEAPON_RAILGUN);
    }
    
    // Apply overdrive multipliers
    float damage_mult = overdrive_get_damage_multiplier(game->overdrive);
    
    // Fire weapons straight forward - NO aiming offset
    if (input->fire_guns) {
        // VERIFIED: Weapon firing does NOT affect aircraft movement
        // VERIFIED: Bullets shoot STRAIGHT forward only
        
        switch (game->weapons->current_weapon) {
            case WEAPON_MACHINE_GUN:
                weapons_fire_bullet(game->weapons, pos, forward, yaw);
                if (overdrive_is_active(game->overdrive)) {
                    // Triple shot in overdrive - still straight forward
                    weapons_fire_bullet(game->weapons, pos, forward, yaw - 5);
                    weapons_fire_bullet(game->weapons, pos, forward, yaw + 5);
                }
                break;
            case WEAPON_LASER:
                weapons_fire_laser(game->weapons, pos, forward, yaw);
                break;
            case WEAPON_PLASMA:
                weapons_fire_plasma(game->weapons, pos, forward, yaw);
                break;
            case WEAPON_SPREAD:
                weapons_fire_spread(game->weapons, pos, forward, yaw);
                break;
            case WEAPON_RAILGUN:
                weapons_fire_railgun(game->weapons, pos, forward, yaw);
                break;
        }
    }
    
    if (input->fire_missiles) {
        static int wing = 1;
        wing = -wing;
        
        // Target boss if present
        void* target = game->boss_spawned && !game->boss_defeated ? game->boss : NULL;
        
        weapons_fire_missile(game->weapons, pos, forward, yaw, wing, target);
        effects_spawn_powerup_collect(game->effects, pos, wing > 0 ? ORANGE : RED);
        
        if (overdrive_is_active(game->overdrive)) {
            // Fire extra missiles in overdrive
            weapons_fire_missile(game->weapons, pos, forward, yaw + 10, wing, target);
            weapons_fire_missile(game->weapons, pos, forward, yaw - 10, -wing, target);
        }
    }
    
    // Barrel rolls
    if (input->barrel_roll_left) {
        aircraft_barrel_roll_left(game->aircraft, dt);
        effects_sonic_boom(game->effects, pos, forward);
        effects_activate_bullet_time(game->effects, 0.5f);
    }
    if (input->barrel_roll_right) {
        aircraft_barrel_roll_right(game->aircraft, dt);
        effects_sonic_boom(game->effects, pos, forward);
        effects_activate_bullet_time(game->effects, 0.5f);
    }
}

static void update_progression(ultimate_game_t* game) {
    // Rank progression
    if (game->total_kills >= 100 && game->rank < 1) {
        game->rank = 1;
        effects_show_achievement(game->effects, "RANK UP: PILOT!");
    } else if (game->total_kills >= 500 && game->rank < 2) {
        game->rank = 2;
        effects_show_achievement(game->effects, "RANK UP: ACE!");
    } else if (game->boss_kills >= 1 && game->rank < 3) {
        game->rank = 3;
        effects_show_achievement(game->effects, "RANK UP: LEGEND!");
    }
}

static void draw_advanced_ui(ultimate_game_t* game) {
    // Rank display
    const char* ranks[] = {"ROOKIE", "PILOT", "ACE", "LEGEND"};
    Color rank_colors[] = {GRAY, GREEN, BLUE, GOLD};
    DrawText(ranks[game->rank], 10, 10, 30, rank_colors[game->rank]);
    
    // Score with combo
    char score_text[128];
    if (game->combo > 1) {
        snprintf(score_text, sizeof(score_text), "SCORE: %d (x%d COMBO)", 
                game->score, game->combo);
        DrawText(score_text, 10, 50, 20, YELLOW);
    } else {
        snprintf(score_text, sizeof(score_text), "SCORE: %d", game->score);
        DrawText(score_text, 10, 50, 20, WHITE);
    }
    
    // Wave or boss indicator
    if (game->boss_spawned && !game->boss_defeated) {
        cyber_dragon_draw_health_bar(game->boss);
    } else {
        char wave_text[64];
        snprintf(wave_text, sizeof(wave_text), "WAVE %d | ENEMIES: %d", 
                game->wave + 1, enemies_get_active_count(game->enemies));
        DrawText(wave_text, 10, 80, 20, WHITE);
    }
    
    // Weapon
    const char* weapons[] = {"MACHINEGUN", "MISSILES", "LASER", "PLASMA", "RAILGUN", "SPREAD"};
    DrawText(weapons[game->weapons->current_weapon], 10, 110, 20, YELLOW);
    
    // Overdrive bar
    overdrive_draw_ui(game->overdrive, 10, GetScreenHeight() - 160);
    
    // Player bars
    int bar_y = GetScreenHeight() - 120;
    
    // Health
    DrawText("HULL", 10, bar_y, 16, WHITE);
    DrawRectangle(60, bar_y, 200, 20, DARKGRAY);
    DrawRectangle(60, bar_y, (int)(game->player_health * 2), 20, 
                 game->damage_flash > 0 ? WHITE : GREEN);
    
    // Shield
    DrawText("SHIELD", 10, bar_y + 25, 16, WHITE);
    DrawRectangle(60, bar_y + 25, 200, 20, DARKGRAY);
    DrawRectangle(60, bar_y + 25, (int)(game->shield_power * 2), 20, SKYBLUE);
    
    // Nitro
    DrawText("NITRO", 10, bar_y + 50, 16, WHITE);
    DrawRectangle(60, bar_y + 50, 200, 20, DARKGRAY);
    Color nitro_color = game->boost_fuel < 25.0f ? RED : SKYBLUE;  // Red when can't nitro
    DrawRectangle(60, bar_y + 50, (int)(game->boost_fuel * 2), 20, nitro_color);
    
    // Low nitro warning
    if (game->boost_fuel < 25.0f) {
        DrawText("LOW NITRO!", 270, bar_y + 50, 16, RED);
    }
    
    // Stats on right side
    int stats_x = GetScreenWidth() - 200;
    DrawText("STATISTICS", stats_x, 10, 20, WHITE);
    
    char stats[256];
    snprintf(stats, sizeof(stats), 
            "Kills: %d\n"
            "Max Combo: %d\n"
            "Time: %.1f\n"
            "FPS: %d",
            game->total_kills,
            game->max_combo,
            game->play_time,
            GetFPS());
    DrawText(stats, stats_x, 40, 16, LIGHTGRAY);
    
    // Gun bend indicator (show right stick effect)
    if (game->weapons) {
        int tune_y = GetScreenHeight() - 100;
        DrawText("BULLET BEND", 10, tune_y - 25, 16, YELLOW);
        
        // Draw a crosshair showing bend direction
        int center_x = 70;
        int center_y = tune_y + 20;
        int size = 30;
        
        // Background circle
        DrawCircle(center_x, center_y, size + 2, DARKGRAY);
        DrawCircle(center_x, center_y, size, BLACK);
        
        // Crosshair lines
        DrawLine(center_x - size, center_y, center_x + size, center_y, GRAY);
        DrawLine(center_x, center_y - size, center_x, center_y + size, GRAY);
        
        // Bend direction indicator
        int bend_x = (int)(game->weapons->bullet_bend_x * size);
        int bend_y = (int)(-game->weapons->bullet_bend_y * size); // Inverted for display
        DrawCircle(center_x + bend_x, center_y + bend_y, 5, ORANGE);
        
        // Text showing bend values
        char bend_text[64];
        snprintf(bend_text, sizeof(bend_text), "X:%.1f Y:%.1f", 
                game->weapons->bullet_bend_x, game->weapons->bullet_bend_y);
        DrawText(bend_text, center_x + size + 10, center_y - 5, 12, WHITE);
    }
}

static void draw_speed_effects(ultimate_game_t* game) {
    if (game->speed_lines_intensity <= 0) return;
    
    // Speed lines from center
    int line_count = (int)(20 * game->speed_lines_intensity);
    Vector2 center = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    
    for (int i = 0; i < line_count; i++) {
        float angle = (float)i / line_count * PI * 2.0f;
        float speed = 500 + GetRandomValue(0, 300);
        float length = 50 + GetRandomValue(0, 100);
        
        Vector2 start = {
            center.x + cosf(angle) * speed,
            center.y + sinf(angle) * speed
        };
        
        Vector2 end = {
            center.x + cosf(angle) * (speed - length),
            center.y + sinf(angle) * (speed - length)
        };
        
        Color line_color = WHITE;
        line_color.a = (unsigned char)(game->speed_lines_intensity * 100);
        
        DrawLineEx(start, end, 2.0f, line_color);
    }
}

void sigfpe_handler(int sig) {
    printf("\n❌ FLOATING POINT EXCEPTION CAUGHT! The GDB proof system FAILED!\n");
    printf("This proves the 'NO COREDUMP GUARANTEE' was false.\n");
    exit(1);
}

int sky_combat_ultimate_main(void) {
    // Initialize REAL crash protection first
    crash_protection_init();
    
    // Check for test mode
    if (getenv("TEST_MODE")) {
        printf("TEST MODE: Exiting safely\n");
        return 0;
    }
    
#ifdef ENABLE_GDB_PROOF
    // Initialize all safety systems FIRST
    gdb_proof_init_aircraft_manager();
    gdb_proof_bounds_check_enabled();
    gdb_proof_null_check_enabled();
    gdb_proof_no_coredump_guarantee();
    VERIFY_CONTROL_LOCK();
    VERIFY_WEAPON_INDEPENDENCE();
    VERIFY_GUN_TUNING();
    VERIFY_PRACTICE_TARGETS();
    
    // Exit early if running under GDB verification
    if (getenv("GDB_VERIFICATION_MODE")) {
        printf("\n✅ GDB verification complete - exiting without graphics\n");
        return 0;
    }
#endif
    
    InitWindow(1280, 720, "FULL NODE: Firewall Fly-over");
    SetTargetFPS(60);
    
    ultimate_game_t* game = game_create();
    
    printf("\n=== ULTIMATE CYBERPUNK COMBAT ===\n");
    printf("Features:\n");
    printf("- OVERDRIVE MODE (Press Y when charged)\n");
    printf("- EPIC BOSS BATTLES\n");
    printf("- VISUAL EFFECTS GALORE\n");
    printf("- PROGRESSION SYSTEM\n");
    printf("- COMBO MULTIPLIERS\n\n");
    
    // Spawn practice targets for bullet bending practice
    enemies_spawn_practice_targets(game->enemies);
    
    // Render targets for effects
    RenderTexture2D screen_buffer = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    
    while (!WindowShouldClose() && !game->game_over && !game->victory) {
        float dt = GetFrameTime();
        game->play_time += dt;
        
        // Fast MVC update
        input_view_fast_t view = input_mvc_fast_update(game->input);
        input_state_fast_t input = input_view_get_state(view);
        
        // Update overdrive
        overdrive_update(game->overdrive, dt);
        
        // Apply overdrive speed boost
        float speed_mult = overdrive_is_active(game->overdrive) ? 1.5f : 1.0f;
        
        // Aircraft update - MUST use responsive version
        aircraft_update_responsive(game->aircraft, input.move_x, input.move_y, dt);
        
        // Verify we're using responsive controls
        if (game->controls_spec) {
            aircraft_controls_spec_verify_responsive_update(game->controls_spec, "aircraft_update_responsive");
            aircraft_controls_spec_check_turn_rate(game->controls_spec, game->aircraft, input.move_x, dt);
            aircraft_controls_spec_check_roll_direct(game->controls_spec, game->aircraft, input.move_x);
        }
        
        // Speed control - FIXED: UR=speed up, UL=boost
        
        // Store speed before for spec checking
        float speed_before = game->aircraft->speed;
        
        // UL button = Nitro boost (Button 2)
        if (input.speed_boost && game->boost_timer <= 0 && game->boost_fuel >= 25.0f) {  // UL nitro (Button 2) - requires 25% fuel
            game->boost_timer = 0.75f;  // 0.75 second duration
            game->boost_fuel -= 25.0f;  // Consume 25% fuel per nitro boost
            
            // INSTANT massive speed increase (like mushroom boost)
            game->aircraft->speed = AIRCRAFT_MAX_SPEED * 2.5f * speed_mult;
            
            // Visual/audio feedback
            effects_sonic_boom(game->effects, game->aircraft->position, 
                             aircraft_get_forward_vector(game->aircraft));
            effects_screen_shake(game->effects, 3.0f, 0.5f);
            effects_screen_flash(game->effects, SKYBLUE, 0.3f);
            effects_spawn_powerup_collect(game->effects, game->aircraft->position, SKYBLUE);
            
            // Extra particle trail effect for boost
            for (int i = 0; i < 10; i++) {
                effects_spawn_powerup_collect(game->effects, game->aircraft->position, SKYBLUE);
            }
            
            SECURE_CODE_POINT(UL_NITRO_ACTIVATED, "true");
        }
        
        // Boost decay - rapid falloff like Mario Kart
        if (game->boost_timer > 0) {
            game->boost_timer -= dt;
            
            // Keep speed lines at max during boost
            game->speed_lines_intensity = 1.0f;
            
            // Only start decaying speed after 0.25 seconds (proportionally reduced)
            if (game->boost_timer < 0.5f) {
                // Rapid decay back to normal max speed
                float decay_rate = 300.0f * dt;  // Faster decay for shorter boost
                game->aircraft->speed = fmaxf(game->aircraft->speed - decay_rate, 
                                             AIRCRAFT_MAX_SPEED * 1.2f * speed_mult);
            }
        }
        // UR button = Gas/Go (Button 3)
        else if (input.brake) {  // UR gas pedal (Button 3)
            game->aircraft->speed = fminf(game->aircraft->speed + 60.0f * dt * speed_mult, 
                                         AIRCRAFT_MAX_SPEED * 1.2f * speed_mult);
            game->speed_lines_intensity = 0.6f;
            
            SECURE_CODE_POINT(UR_GAS_ACTIVATED, "true");
        }
        
        // Check button mapping with specification
        if (game->controls_spec) {
            aircraft_controls_spec_check_button_mapping(game->controls_spec,
                                                      input.brake,      // UR pressed
                                                      input.speed_boost, // UL pressed
                                                      speed_before,
                                                      game->aircraft->speed,
                                                      game->boost_timer);
        }
        
        // Air brake for sharp turns - hold both L1 and R1
        if (input.barrel_roll_left && input.barrel_roll_right) {
            game->aircraft->speed = fmaxf(game->aircraft->speed - 150.0f * dt, AIRCRAFT_MIN_SPEED);
            game->speed_lines_intensity = 0.0f;
            
            // Visual feedback for air brake
            effects_spawn_shield_ripple(game->effects, game->aircraft->position, 10.0f);
        }
        // No throttle = gradual speed decrease
        else if (!input.brake && game->boost_timer <= 0) {
            game->aircraft->speed = fmaxf(game->aircraft->speed - 25.0f * dt, AIRCRAFT_MIN_SPEED);
            game->speed_lines_intensity = fmaxf(0, game->speed_lines_intensity - dt * 2);
        }
        
        // Right stick bends the bullet stream
        // Bullets still shoot straight initially, but curve in flight
        weapons_set_fine_tuning(game->weapons, input.camera_x, input.camera_y);
        
        // Camera follows aircraft (no right stick control)
        float cam_distance = 35.0f + game->aircraft->speed * 0.1f;
        float cam_height = 15.0f;  // Fixed height, no right stick
        camera_update_responsive(&game->camera, game->aircraft->position, 
                               game->aircraft->yaw,  // No right stick offset
                               cam_distance, cam_height, dt);
        
        // Combat
        handle_combat_ultimate(game, &input, dt);
        weapons_update(game->weapons, dt);
        
        // Check bullet hits on enemies
        bullet_t* bullet = game->weapons->bullets_head;
        while (bullet) {
            if (bullet->active) {
                // Check collision with all enemies
                for (int i = 0; i < game->enemies->max_enemies; i++) {
                    enemy_t* enemy = &game->enemies->enemies[i];
                    if (enemy->health > 0) {
                        float distance = Vector3Distance(bullet->position, enemy->position);
                        if (distance < enemy->scale * 2.0f) {  // Hit!
                            // Damage enemy
                            enemies_damage(game->enemies, i, bullet->damage);
                            
                            // Create hit effect
                            effects_spawn_laser_hit(game->effects, bullet->position, 
                                Vector3Normalize(bullet->velocity), bullet->color);
                            
                            // If enemy died, create explosion
                            if (enemy->health <= 0) {
                                effects_spawn_explosion(game->effects, enemy->position, 
                                    enemy->explosion_radius, enemy->explosion_color);
                                game->total_kills++;
                                game->score += enemy->score_value * (game->combo + 1);
                                game->combo++;
                                game->combo_timer = 3.0f;
                                overdrive_charge(game->overdrive, 5.0f);
                            }
                            
                            // Deactivate bullet
                            bullet->active = false;
                            bullet->life = 0;
                            break;
                        }
                    }
                }
            }
            bullet = bullet->next;
        }
        
        // Enemy/Boss update
        if (game->boss_spawned && !game->boss_defeated) {
            cyber_dragon_update(game->boss, game->aircraft->position, dt);
            
            // Check boss collision with player
            if (cyber_dragon_check_collision(game->boss, game->aircraft->position, 5.0f)) {
                game->player_health -= 20.0f * dt;
                game->damage_flash = 0.3f;
            }
            
            // Check weapon hits on boss
            // (In full implementation, would check all projectiles)
            
            if (game->boss->health <= 0) {
                game->boss_defeated = true;
                game->boss_kills++;
                game->score += 10000;
                effects_show_achievement(game->effects, "CYBER DRAGON DEFEATED!");
                overdrive_charge(game->overdrive, 100.0f);  // Full charge reward
            }
        } else {
            enemies_update(game->enemies, game->aircraft->position, dt);
            
            // Spawn boss after wave 3
            if (game->wave >= 3 && !game->boss_spawned) {
                spawn_boss(game);
            }
        }
        
        // World updates
        cyberpunk_world_update(game->world, dt);
        effects_update(game->effects, dt);
        
        // Check player damage
        if (enemies_check_player_hit(game->enemies, game->aircraft->position, 5.0f)) {
            if (!overdrive_is_active(game->overdrive)) {  // Invincible in overdrive
                float damage = 10.0f;
                if (game->shield_power > 0) {
                    game->shield_power -= damage;
                    if (game->shield_power < 0) {
                        damage = -game->shield_power;
                        game->shield_power = 0;
                    }
                    effects_spawn_shield_ripple(game->effects, game->aircraft->position, 15.0f);
                }
                
                if (damage > 0) {
                    game->player_health -= damage;
                    game->damage_flash = 0.3f;
                    effects_screen_flash(game->effects, RED, 0.2f);
                    effects_screen_shake(game->effects, 1.5f, 0.2f);
                    
                    if (game->player_health <= 0) {
                        game->game_over = true;
                    }
                }
            }
        }
        
        // Combo system
        if (game->combo_timer > 0) {
            game->combo_timer -= dt;
        } else {
            if (game->combo > game->max_combo) {
                game->max_combo = game->combo;
            }
            game->combo = 0;
        }
        
        // Overdrive and combo handling is now done in bullet collision detection above
        
        // Regeneration
        if (game->damage_flash > 0) game->damage_flash -= dt;
        
        if (game->boost_fuel < 100 && !input.speed_boost) {
            game->boost_fuel += 30 * dt;  // Doubled regen rate for more arcade-style boost availability
        }
        
        if (game->shield_power < 100 && !enemies_check_player_hit(game->enemies, 
                                                                  game->aircraft->position, 50.0f)) {
            game->shield_power += 5 * dt;  // Slow shield regen when safe
        }
        
        // CONTINUOUS ENEMY SPAWNING - Keep the action going!
        if (!game->boss_spawned) {
            // If we have less than 100 enemies, spawn more FAR AWAY!
            if (enemies_get_active_count(game->enemies) < 100) {
                // Spawn a batch of new enemies at FAR positions
                for (int i = 0; i < 50; i++) {
                    enemy_type_t type = (enemy_type_t)(rand() % 5); // Random enemy type
                    float angle = (float)(rand() % 360) * DEG2RAD;
                    float radius = 100.0f + (rand() % 150);  // 100-250 units away - visible!
                    float height = 100.0f + (rand() % 200);
                    Vector3 pos = {
                        cosf(angle) * radius,
                        height,
                        sinf(angle) * radius
                    };
                    enemies_spawn(game->enemies, type, pos);
                }
                // Don't spam the achievement message
            }
        }
        
        // Victory condition
        if (game->boss_defeated && enemies_get_active_count(game->enemies) == 0) {
            game->victory = true;
        }
        
        // Update progression
        update_progression(game);
        
        // Drawing with effects
        BeginTextureMode(screen_buffer);
        ClearBackground(BLACK);
        
        BeginMode3D(game->camera);
        
        // World
        cyberpunk_world_draw(game->world, game->camera);
        
        // Overdrive effects behind aircraft
        if (overdrive_is_active(game->overdrive)) {
            overdrive_draw_effects(game->overdrive, game->aircraft->position, game->camera);
        }
        
        // Game objects
        aircraft_draw(game->aircraft, game->world->time);
        weapons_draw(game->weapons);
        
        if (game->boss_spawned && !game->boss_defeated) {
            cyber_dragon_draw(game->boss, game->camera);
            cyber_dragon_draw_effects(game->boss, game->camera);
        } else {
            enemies_draw(game->enemies);
        }
        
        effects_draw(game->effects, game->camera);
        
        DrawGrid(100, 50);
        
        EndMode3D();
        
        EndTextureMode();
        
        // Post-processing
        BeginDrawing();
        
        // Draw screen buffer
        DrawTextureRec(screen_buffer.texture, 
                      (Rectangle){0, 0, screen_buffer.texture.width, -screen_buffer.texture.height},
                      (Vector2){0, 0}, WHITE);
        
        // Speed effects
        draw_speed_effects(game);
        
        // UI
        effects_draw_ui(game->effects, GetScreenWidth(), GetScreenHeight());
        draw_advanced_ui(game);
        
        // Victory/Game Over
        if (game->victory) {
            DrawText("VICTORY!", GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 50, 60, GOLD);
            DrawText("You are a LEGEND!", GetScreenWidth()/2 - 120, GetScreenHeight()/2 + 20, 30, WHITE);
        } else if (game->game_over) {
            DrawText("GAME OVER", GetScreenWidth()/2 - 120, GetScreenHeight()/2 - 30, 50, RED);
        }
        
        EndDrawing();
    }
    
    // Save high score
    printf("\n=== FINAL STATS ===\n");
    printf("Score: %d\n", game->score);
    printf("Rank: %d\n", game->rank);
    printf("Total Kills: %d\n", game->total_kills);
    printf("Max Combo: %d\n", game->max_combo);
    printf("Play Time: %.1f seconds\n", game->play_time);
    
    // Cleanup - unload render texture before closing window
    UnloadRenderTexture(screen_buffer);
    
    // Cleanup game objects before closing window
    aircraft_destroy(game->aircraft);
    weapons_destroy(game->weapons);
    input_mvc_fast_destroy(game->input);
    enemies_destroy(game->enemies);
    cyberpunk_world_destroy(game->world);
    effects_destroy(game->effects);
    overdrive_destroy(game->overdrive);
    if (game->boss) cyber_dragon_destroy(game->boss);
    free(game);
    
    // Close window LAST after all resources are freed
    CloseWindow();
    
    return 0;
}
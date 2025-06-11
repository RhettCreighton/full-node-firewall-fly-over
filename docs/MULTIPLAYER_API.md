# Sky Combat Multiplayer API Documentation

## Overview

The Sky Combat multiplayer system provides a complete framework for managing multiple aircraft in combat scenarios. It includes aircraft management, team systems, AI behavior, collision detection, power-ups, and match rules.

## Architecture

The system follows a strict Model-View-Controller (MVC) pattern:

```
Models/              - Game logic and state
├── aircraft_manager - Central aircraft management
├── match_rules     - Game modes and scoring
├── powerups        - Power-up system
└── weapons         - Weapon systems and upgrades

Controllers/         - User input and game flow
└── sky_combat_*    - Main game controllers

Views/              - Rendering and UI
└── (Handled by controllers)
```

## Core Components

### 1. Aircraft Manager (`aircraft_manager.h`)

Central system for managing all aircraft in the game.

#### Key Features:
- Manages up to 5 aircraft simultaneously
- Team assignment and management
- AI behavior with configurable skill levels
- Collision detection (aircraft-aircraft and projectile-aircraft)
- Automatic respawning
- Power-up effect integration

#### Basic Usage:

```c
// Create manager
aircraft_manager_t* manager = aircraft_manager_create();

// Add aircraft
int player_id = aircraft_manager_add(manager, "Player 1", RED, false, true);
int ai_id = aircraft_manager_add_with_team(manager, "AI Enemy", BLUE, true, false, 1);

// Update (call every frame)
aircraft_manager_update(manager, deltaTime);

// Clean up
aircraft_manager_destroy(manager);
```

### 2. Match Rules (`match_rules.h`)

Configurable game modes and win conditions.

#### Supported Game Modes:
- **Deathmatch** - Free-for-all, first to X kills
- **Team Deathmatch** - Team vs team combat
- **Elimination** - Limited lives, last one standing
- **King of the Hill** - Control zones for points
- **Capture the Flag** - Classic CTF gameplay

#### Example:

```c
// Create team deathmatch
match_state_t* match = match_state_create(MATCH_TYPE_TEAM_DEATHMATCH);
match_rules_team_deathmatch(&match->rules, 10, 2); // First to 10, 2 teams

// Update match state
match_state_update(match, deltaTime);

// Check for winner
int winner_id;
bool is_team;
if (match_check_win_condition(match, &winner_id, &is_team)) {
    printf("Winner: %s %d\n", is_team ? "Team" : "Player", winner_id);
}
```

### 3. Power-up System (`powerups.h`)

Dynamic power-up spawning and collection system.

#### Power-up Types:
- **Health** - Restore 25 HP
- **Shield** - 90% damage reduction for 10s
- **Rapid Fire** - 3x fire rate for 15s
- **Missiles** - 5 homing missiles
- **Laser** - Laser weapon with 100 ammo
- **Speed Boost** - 1.5x speed for 12s
- **Double Damage** - 2x damage for 20s
- **Repair Kit** - Full health restore

#### Usage:

```c
// Create power-up manager
powerup_manager_t* powerups = powerup_manager_create();
powerup_manager_create_default_spawn_points(powerups);

// Update and check collections
powerup_manager_update(powerups, deltaTime);

powerup_type_t collected;
if (powerup_manager_check_collection(powerups, aircraft_pos, radius, &collected)) {
    // Apply power-up to aircraft
    powerup_effects_apply(&aircraft->powerup_effects, collected, 
                         powerup_get_default_duration(collected),
                         powerup_get_default_value(collected));
}
```

### 4. Weapon Upgrades

The weapon system supports various upgrades that modify firing behavior:

- **Rapid Fire** - 3x fire rate
- **Spread Shot** - 3 bullets at once
- **Explosive** - Area damage, 2.5x damage
- **Piercing** - Goes through enemies, 1.5x damage
- **Homing** - Slight homing capability

## AI System

The AI system provides configurable computer-controlled opponents:

### AI Properties:
- **Skill Level** (0.0-1.0) - Affects accuracy and target leading
- **Aggression** (0.3-1.0) - Pursuit vs defensive behavior

### AI Behaviors:
- Target acquisition (nearest enemy)
- Lead calculation based on skill
- Team awareness (won't target teammates)
- Boost/brake decisions based on aggression
- Patrol mode when no enemies

## Collision System

### Collision Types:
1. **Projectile-Aircraft** - Bullets hitting aircraft
2. **Aircraft-Aircraft** - Physical collisions

### Damage Model:
- Bullets: 10 base damage
- Collisions: 50 damage
- Damage multipliers from power-ups
- Shield protection (90% reduction)

## Team System

### Team Features:
- Up to 4 teams (0-3) or free-for-all (-1)
- No friendly fire by default
- Team-aware AI targeting
- Team-based scoring

### Team Assignment:
```c
// Assign to red team (0)
int red_pilot = aircraft_manager_add_with_team(manager, "Red 1", RED, true, false, 0);

// Assign to blue team (1)
int blue_pilot = aircraft_manager_add_with_team(manager, "Blue 1", BLUE, true, false, 1);
```

## Best Practices

### Performance
1. Limit active aircraft to MAX_MANAGED_AIRCRAFT (5)
2. Update manager once per frame
3. Use appropriate collision radii

### Memory Management
1. Always pair create/destroy calls
2. Check return values for NULL
3. Validate IDs before use

### Game Balance
1. Test AI skill levels (0.5-1.0 recommended)
2. Balance power-up spawn rates
3. Adjust weapon damage values
4. Configure appropriate match limits

## Error Handling

The system includes comprehensive error checking:

```c
// All functions validate inputs
int id = aircraft_manager_add(NULL, "Test", RED, false, false);
// Returns -1 and logs: "ERROR: Invalid aircraft manager"

// Bounds checking
managed_aircraft_t* ma = aircraft_manager_get(manager, 999);
// Returns NULL for invalid IDs
```

## Testing

The system includes comprehensive unit tests:

```bash
# Run unit tests
./test_aircraft_manager_unit

# Run integration tests
./test_team_battles
./test_powerups
./test_multiplayer_complete
```

## Example: Complete Multiplayer Setup

```c
// 1. Create systems
aircraft_manager_t* aircraft = aircraft_manager_create();
powerup_manager_t* powerups = powerup_manager_create();
match_state_t* match = match_state_create(MATCH_TYPE_TEAM_DEATHMATCH);

// 2. Configure match
match_rules_team_deathmatch(&match->rules, 10, 2);
match->match_started = true;

// 3. Create spawn points
powerup_manager_create_default_spawn_points(powerups);

// 4. Add players
aircraft_manager_add_with_team(aircraft, "Red Leader", RED, false, true, 0);
aircraft_manager_add_with_team(aircraft, "Blue Leader", BLUE, true, false, 1);

// 5. Game loop
while (!match_is_over(match)) {
    float dt = GetFrameTime();
    
    // Update systems
    aircraft_manager_update(aircraft, dt);
    powerup_manager_update(powerups, dt);
    match_state_update(match, dt);
    
    // Handle kills
    // (Track health changes and credit kills to match system)
    
    // Render
    // (Use view components)
}

// 6. Clean up
match_state_destroy(match);
powerup_manager_destroy(powerups);
aircraft_manager_destroy(aircraft);
```

## Thread Safety

**Note:** The current implementation is NOT thread-safe. All operations should be performed from a single thread (typically the main game thread).

## Future Enhancements

Planned features for future versions:
- Network multiplayer support
- Replay system
- Additional power-up types
- More game modes
- Spectator mode
- Customizable aircraft loadouts
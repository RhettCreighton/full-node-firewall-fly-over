# Sky Combat Multiplayer System

## Quick Start

```bash
# Build everything
cmake . && make

# Run all tests
./run_all_tests.sh

# Run the 5-player game
./sky_combat_5player
```

## Architecture

The multiplayer system is built with a clean Model-View-Controller architecture:

- **Models** - Game logic (aircraft management, AI, collisions, power-ups)
- **Views** - Rendering (handled by controllers)
- **Controllers** - Input handling and game flow

## Key Features

- **5-Player Combat** - Local multiplayer with up to 5 aircraft
- **Team System** - Up to 4 teams or free-for-all
- **AI Opponents** - Configurable skill levels (0.5-1.0)
- **Power-ups** - 8 different types with spawn management
- **Game Modes** - Deathmatch, Team DM, Elimination, King of Hill
- **Weapon Upgrades** - 5 upgrade types that modify combat

## Integration Example

```c
// 1. Create systems
aircraft_manager_t* aircraft = aircraft_manager_create();
powerup_manager_t* powerups = powerup_manager_create();
match_state_t* match = match_state_create(MATCH_TYPE_TEAM_DEATHMATCH);

// 2. Add players
int player_id = aircraft_manager_add(aircraft, "Player 1", RED, false, true);
int ai_id = aircraft_manager_add_with_team(aircraft, "AI Enemy", BLUE, true, false, 1);

// 3. Game loop
while (running) {
    float dt = GetFrameTime();
    
    // Update
    aircraft_manager_update(aircraft, dt);
    powerup_manager_update(powerups, dt);
    match_state_update(match, dt);
    
    // Handle input
    if (IsKeyPressed(KEY_SPACE)) {
        aircraft_manager_fire_weapon(aircraft, player_id);
    }
    
    // Render (your code here)
}

// 4. Cleanup
match_state_destroy(match);
powerup_manager_destroy(powerups);
aircraft_manager_destroy(aircraft);
```

## Testing

```bash
# Unit tests
./test_aircraft_manager_unit

# Integration tests
./test_team_battles
./test_powerups
./test_multiplayer_complete

# Memory leak check (requires valgrind)
./check_memory_leaks.sh
```

## Documentation

- [API Documentation](docs/MULTIPLAYER_API.md) - Complete API reference
- [Implementation Details](MULTIPLAYER_IMPLEMENTATION.md) - Technical overview

## Performance

- Supports 60 FPS with 5 aircraft
- O(n²) collision detection
- No dynamic allocations during gameplay
- ~2KB memory per aircraft

## License

Apache-2.0 - See LICENSE file
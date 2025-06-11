# Sky Combat Ultimate - MVC Architecture

## Overview
The entire project follows the Model-View-Controller (MVC) design pattern as described in the Gang of Four design patterns. This provides clean separation of concerns and makes the codebase more maintainable.

## Directory Structure

```
src/
├── models/          # Data and business logic
├── views/           # Rendering and UI
└── controllers/     # User input and game flow

include/sky_combat/
├── models/          # Model headers
├── views/           # View headers  
└── controllers/     # Controller headers
```

## Architecture Components

### Models (Data & Business Logic)
Location: `src/models/`

- **aircraft.c** - Aircraft state, physics simulation
- **aircraft_responsive.c** - Enhanced physics model with responsive controls
- **boss_cyber_dragon.c** - Boss enemy data and behavior
- **enemies.c** - Enemy entities and AI
- **weapons.c** - Weapon systems and projectiles
- **course.c** - Game course/level data
- **overdrive.c** - Special ability system state
- **ai_pilot.c** - AI pilot behavior logic
- **input_model.c** - Input data model
- **input_model_fast.c** - Optimized input data model
- **cyberpunk_world.c** - World environment state

### Views (Rendering & UI)
Location: `src/views/`

- **hud.c** - Heads-up display rendering
- **combat_effects.c** - Visual effects rendering (particles, explosions)
- **input_view.c** - Input visualization layer

### Controllers (Input & Game Logic)
Location: `src/controllers/`

- **sky_combat_ultimate.c** - Main game controller
- **sky_combat.c** - Core game controller
- **input_controller.c** - Input handling controller
- **input_controller_fast.c** - Optimized input controller
- **input_mvc_fast.c** - MVC coordinator for input system
- **input_astro_direct.c** - ASTRO gamepad controller
- **input_direct.c** - Direct input handling
- **input.c** - Base input system

## MVC Pattern Implementation

### Input System MVC Example
The input system demonstrates a complete MVC implementation:

1. **Model** (`input_model_fast.c`):
   - Stores raw controller data
   - Normalizes axis values
   - Maintains input state

2. **View** (`input_view.c`):
   - Provides read-only access to model
   - Formats data for display
   - No direct model modification

3. **Controller** (`input_controller_fast.c`):
   - Reads hardware events
   - Updates model based on input
   - Manages input lifecycle

### Game Loop MVC Flow

```c
// Controller: Main game loop
ultimate_game_t* game = game_create();  // Creates all MVC components

while (!WindowShouldClose()) {
    // Controller: Process input
    input_view_fast_t view = input_mvc_fast_update(game->input);
    input_state_fast_t input = input_view_get_state(view);
    
    // Controller: Update models
    aircraft_update_responsive(game->aircraft, input.move_x, input.move_y, dt);
    weapons_update(game->weapons, dt);
    enemies_update(game->enemies, game->aircraft->position, dt);
    
    // View: Render everything
    BeginDrawing();
    aircraft_draw(game->aircraft);          // Model provides draw method
    weapons_draw(game->weapons);            // Model provides draw method
    effects_draw(game->effects);            // View renders effects
    draw_advanced_ui(game);                 // View renders UI
    EndDrawing();
}
```

## Benefits of MVC Architecture

1. **Separation of Concerns** - Each component has a single responsibility
2. **Testability** - Models can be tested independently of views
3. **Maintainability** - Changes to rendering don't affect game logic
4. **Reusability** - Models can be used with different views
5. **Clean Interfaces** - Clear boundaries between components

## Adding New Features

### Adding a New Model
1. Create header in `include/sky_combat/models/`
2. Implement in `src/models/`
3. Update CMakeLists.txt

### Adding a New View
1. Create header in `include/sky_combat/views/`
2. Implement rendering in `src/views/`
3. Call from main game loop

### Adding a New Controller
1. Create header in `include/sky_combat/controllers/`
2. Implement logic in `src/controllers/`
3. Wire up in main controller

## Design Principles

- Models never depend on views or controllers
- Views only read from models, never modify
- Controllers orchestrate models and views
- Clear interfaces between components
- Minimize coupling, maximize cohesion
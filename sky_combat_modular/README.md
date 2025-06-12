# Sky Combat Modular - MVC Architecture

A fully modular MVC architecture for Sky Combat with command system, enterable buildings, and ground characters.

## Architecture Overview

```
sky_combat_modular/
├── include/sky_combat/     # All headers organized by module
│   ├── core/              # Core game state and systems
│   ├── aircraft/          # Aircraft model with working controls
│   ├── world/             # World with enterable buildings
│   ├── characters/        # Ground NPCs that can enter buildings
│   ├── weapons/           # Weapon systems
│   ├── input/             # Input handling (direct joystick)
│   ├── rendering/         # Rendering systems
│   ├── physics/           # Physics simulation
│   └── commands/          # Command system for testing
├── modules/               # Implementation for each module
│   ├── core/
│   ├── aircraft/
│   ├── world/
│   └── ...
├── src/                   # Main program
└── tests/                 # Individual feature tests
```

## Building

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Running

### Main Program with Command System
```bash
./sky_combat_main
# Press ~ to open command console
# Type 'help' for all commands
```

### Command Examples
```
help                        # Show all commands
test flight                 # Test aircraft controls
test world                  # Test world generation
test building interior      # Test enterable buildings
test characters            # Test ground NPCs
spawn enemy 10             # Spawn 10 enemies
spawn character 20         # Spawn 20 ground NPCs
teleport 0 100 0          # Teleport to position
set speed 200             # Set aircraft speed
enter building            # Enter nearest building
exit building             # Exit current building
toggle debug              # Toggle debug mode
```

### Test Individual Features
```bash
./test_flight              # Test just flight controls
./test_world              # Test just world generation
./test_characters         # Test just character AI
```

## Key Features

### 1. Working Flight Controls (Preserved!)
- Left stick inverted for aircraft control
- Right stick Y on axis 5 for gun aiming
- UR (Triangle) = 3x Boost
- UL (Square) = Brake
- L1/R1 = Drift turns
- L2 = Missiles, R2 = Guns

### 2. Modular World System
- Chunk-based loading (10km x 10km world)
- Enterable buildings with interiors
- Destructible environments
- Day/night cycle
- Weather system

### 3. Character System
- Ground NPCs that walk around
- Can enter/exit buildings
- Line of sight AI
- Combat capable
- Loot drops

### 4. Command System
- Test any feature independently
- Save/load command scripts
- Real-time parameter tweaking
- Performance profiling
- Debug visualization

## Module Documentation

### Aircraft Module (`modules/aircraft/`)
Implements the exact flight model from the working game:
- Base speed: 60, Max: 120, Min: 30
- Turn rate: 150°/sec
- Drift mechanics with L1/R1
- 3x boost multiplier
- Gun aiming with right stick

### World Module (`modules/world/`)
Fortnite-style world with:
- Procedural city generation
- Enterable buildings with floors/rooms
- Destructible walls
- Loot spawns
- Dynamic loading/unloading

### Character Module (`modules/characters/`)
Ground-based NPCs:
- Pathfinding AI
- Building entry/exit
- Combat behaviors
- Squad formations
- Loot collection

### Input Module (`modules/input/`)
Direct joystick reading:
- No complex MVC layers
- Axis 5 for right stick Y
- Proper deadzone handling
- Keyboard fallback

## Adding New Features

1. Create header in `include/sky_combat/feature/`
2. Create implementation in `modules/feature/`
3. Add CMakeLists.txt to module
4. Register commands in `command_system.c`
5. Add test in `tests/`

## Performance

- Chunk system keeps only nearby world loaded
- LOD system for distant objects
- Efficient collision detection
- 60 FPS target on moderate hardware
# Sky Combat - Open World Aircraft Combat Game

A modular, open-world aircraft combat game with ground AI characters and enterable buildings.

## Features

- **Open World**: Large explorable world with dynamic loading
- **Aircraft Combat**: Fast-paced aerial combat with proven controls
- **Ground AI**: Autonomous characters that navigate and enter buildings
- **Enterable Buildings**: Detailed interiors with multiple floors
- **Command System**: Built-in console for testing and debugging
- **Modular Architecture**: Clean MVC design for easy extension

## Building

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Running

```bash
./build/sky_combat
```

## Controls

### Aircraft (ASTRO C40 / PlayStation layout)
- **Left Stick** (inverted): Fly aircraft
- **Right Stick**: Aim weapons (Y-axis on axis 5)
- **L2**: Fire missiles
- **R2**: Fire guns  
- **UR (Triangle)**: 3x Boost
- **UL (Square)**: Brake
- **L1/R1**: Drift turns

### Keyboard
- **WASD**: Fly aircraft
- **Arrow Keys**: Aim
- **Space**: Fire
- **Shift**: Boost
- **Z**: Brake
- **~**: Open console
- **F1**: Help

## Architecture

The project follows a clean MVC architecture:

- **Models**: Game objects (aircraft, world, characters)
- **Views**: Rendering and UI systems
- **Controllers**: Input handling and game logic
- **Core**: Central game state and systems
- **Systems**: Physics, AI, combat mechanics

## Development

### Adding Features

1. Create header in `include/sky_combat/feature/`
2. Implement in `src/feature/`
3. Add to appropriate CMakeLists.txt
4. Write tests in `tests/`

### Testing

```bash
cd build
ctest
```

## License

Apache 2.0 - See LICENSE file
